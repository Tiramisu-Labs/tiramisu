#include "../include/SshHandler.hpp"

#include <ios>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <cstring>
#include <termios.h>
#include <unistd.h> // For usleep

SshHandler::SshHandler(const std::string& host, const std::string& user, int port, const std::string& key_path)
{
    sshSession = SshSessionPtr(ssh_new());
    if (sshSession) {
        ssh_options_set(sshSession.get(), SSH_OPTIONS_HOST, host.c_str());
        ssh_options_set(sshSession.get(), SSH_OPTIONS_USER, user.c_str());
        ssh_options_set(sshSession.get(), SSH_OPTIONS_PORT, &port);

        long timeout_sec = 5;
        ssh_options_set(sshSession.get(), SSH_OPTIONS_TIMEOUT, &timeout_sec);

        if (!key_path.empty()) {
            ssh_options_set(sshSession.get(), SSH_OPTIONS_IDENTITY, key_path.c_str());
        }
    } else {
        throw std::runtime_error("Failed to allocate memory structure for libssh session.");
    }
}

bool SshHandler::sshConnect() 
{
    if (connected) return true;

    int rc = ssh_connect(sshSession.get());
    if (rc != SSH_OK) {
        std::cerr << "Connection error: " << ssh_get_error(sshSession.get()) << "\n";
        return false;
    }

    if (!verify_knownhost()) {
        std::cerr << "Security validation failed. Aborting connection handshake.\n";
        ssh_disconnect(sshSession.get());
        return false;
    }

    connected = true;
    return true;
}

ssh_channel SshHandler::initChannel() {
    if (!authenticated && !attemptConnection()) {
        std::cerr << "ssh: error: Session must be authenticated before opening a channel.\n";
        return NULL;
    }
    
    ssh_channel channel = ssh_channel_new(sshSession.get());
    if (channel == NULL) {
        std::cerr << "Channel allocation error: " << ssh_get_error(sshSession.get()) << "\n";
        return NULL;
    }
    
    int rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        std::cerr << "Channel session open error: " << ssh_get_error(sshSession.get()) << "\n";
        ssh_channel_free(channel);
        return NULL;
    }
    return channel;
}

std::string SshHandler::getArch()
{
    ssh_channel channel = initChannel();
    if (channel == NULL) {
        std::cerr << "ssh: error: failed to init new channel\n";
        return "";
    }
    
    std::string arch = "";
    if (ssh_channel_request_exec(channel, "uname -m") == SSH_OK) {
        char buffer[256];
        int nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
        if (nbytes > 0) {
            arch.append(buffer, nbytes);
        }
    }
    
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    arch.erase(arch.find_last_not_of(" \n\r\t") + 1);
    return arch;
}

bool SshHandler::exec_remote_command(const std::string& command)
{
    ssh_channel channel = initChannel();
    if (!channel) {
        return false;
    }
    
    int rc = ssh_channel_request_exec(channel, command.c_str());
    if (rc != SSH_OK) {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return false;
    }
    
    char buffer[1024];
    int nbytes;
    
    while (!ssh_channel_is_eof(channel)) {
        if (!ssh_is_connected(sshSession.get())) {
            std::cerr << "[SSH] Connection lost during execution.\n";
            break;
        }

        nbytes = ssh_channel_read_nonblocking(channel, buffer, sizeof(buffer), 0);
        if (nbytes > 0) {
            if (write(1, buffer, nbytes) != nbytes) {}
        } else if (nbytes < 0) {
            break;
        }
        
        nbytes = ssh_channel_read_nonblocking(channel, buffer, sizeof(buffer), 1);
        if (nbytes > 0) {
            if (write(2, buffer, nbytes) != nbytes) {}
        } else if (nbytes < 0) {
            break;
        }
        
        usleep(10000);
    }
    
    while (ssh_is_connected(sshSession.get()) && (nbytes = ssh_channel_read_nonblocking(channel, buffer, sizeof(buffer), 0)) > 0) {
        if (write(1, buffer, nbytes) != nbytes) {}
    }
    while (ssh_is_connected(sshSession.get()) && (nbytes = ssh_channel_read_nonblocking(channel, buffer, sizeof(buffer), 1)) > 0) {
        if (write(2, buffer, nbytes) != nbytes) {}
    }
    
    int exit_status = ssh_channel_get_exit_status(channel);
    
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    
    return (exit_status == 0);
}

bool SshHandler::verify_knownhost()
{
    enum ssh_known_hosts_e state;
    unsigned char *hash = NULL;
    ssh_key srv_pubkey = NULL;
    size_t hlen;
    std::string buf;
    char *hexa;
    int rc;
 
    rc = ssh_get_server_publickey(sshSession.get(), &srv_pubkey);
    if (rc < 0) {
        return false;
    }
 
    rc = ssh_get_publickey_hash(srv_pubkey, SSH_PUBLICKEY_HASH_SHA1, &hash, &hlen);
    ssh_key_free(srv_pubkey);
    if (rc < 0) {
        return false;
    }
 
    state = ssh_session_is_known_server(sshSession.get());
    switch (state) {
        case SSH_KNOWN_HOSTS_OK:
            break;
        case SSH_KNOWN_HOSTS_CHANGED:
            std::cerr << "Host key for server changed.\n";
            ssh_clean_pubkey_hash(&hash);
            return false;
        case SSH_KNOWN_HOSTS_OTHER:
            std::cerr << "An alternative key type exists for this host.\n";
            ssh_clean_pubkey_hash(&hash);
            return false;
        case SSH_KNOWN_HOSTS_NOT_FOUND:
            std::cerr << "Could not find known host file.\n";
            [[fallthrough]];
        case SSH_KNOWN_HOSTS_UNKNOWN:
            hexa = ssh_get_hexa(hash, hlen);
            std::cerr << "The server is unknown. Trust key? (yes/no): ";
            ssh_string_free_char(hexa);
            ssh_clean_pubkey_hash(&hash);
            
            if (!(std::cin >> buf) || buf != "yes") {
                return false;
            }
 
            rc = ssh_session_update_known_hosts(sshSession.get());
            if (rc < 0) {
                std::cerr << "Error updating known hosts file.\n";
                return false;
            }
            break;
        case SSH_KNOWN_HOSTS_ERROR:
            std::cerr << "Error: " << ssh_get_error(sshSession.get()) << "\n";
            ssh_clean_pubkey_hash(&hash);
            return false;
    }
    ssh_clean_pubkey_hash(&hash);
    return true;
}

bool SshHandler::try_password(const std::string& password) 
{
    int rc = ssh_userauth_password(sshSession.get(), nullptr, password.c_str());
    
    if (rc == SSH_AUTH_SUCCESS) {
        authenticated = true;
        return true;
    }

    if (rc == SSH_AUTH_DENIED || rc == SSH_AUTH_PARTIAL) {
        rc = ssh_userauth_kbdint(sshSession.get(), nullptr, nullptr);
        
        while (rc == SSH_AUTH_INFO) {
            int nprompts = ssh_userauth_kbdint_getnprompts(sshSession.get());
            for (int i = 0; i < nprompts; ++i) {
                const char* prompt = ssh_userauth_kbdint_getprompt(sshSession.get(), i, nullptr);
                std::string prompt_str(prompt ? prompt : "");
                
                if (prompt_str.find("password") != std::string::npos || 
                    prompt_str.find("Password") != std::string::npos) {
                    ssh_userauth_kbdint_setanswer(sshSession.get(), i, password.c_str());
                }
            }
            rc = ssh_userauth_kbdint(sshSession.get(), nullptr, nullptr);
        }
        
        if (rc == SSH_AUTH_SUCCESS) {
            authenticated = true;
            return true;
        }
    }
    return false;
}

bool SshHandler::authenticate_kbdint()
{
    int rc = ssh_userauth_kbdint(sshSession.get(), nullptr, nullptr);
    
    while (rc == SSH_AUTH_INFO) 
    {
        const char *name = ssh_userauth_kbdint_getname(sshSession.get());
        const char *instruction = ssh_userauth_kbdint_getinstruction(sshSession.get());
        int nprompts = ssh_userauth_kbdint_getnprompts(sshSession.get());
        
        if (name && std::strlen(name) > 0) std::cout << name << "\n";
        if (instruction && std::strlen(instruction) > 0) std::cout << instruction << "\n";
        
        for (int iprompt = 0; iprompt < nprompts; iprompt++) 
        {
            char echo;
            const char *prompt = ssh_userauth_kbdint_getprompt(sshSession.get(), iprompt, &echo);
            if (!prompt) continue;
            std::cout << prompt << std::flush;
            
            std::string answer;
            if (echo) {
                std::getline(std::cin, answer);
            } else {
                struct termios oldt;
                tcgetattr(STDIN_FILENO, &oldt);
                struct termios newt = oldt;
                newt.c_lflag &= ~ECHO;
                tcsetattr(STDIN_FILENO, TCSANOW, &newt);
                
                std::getline(std::cin, answer);
                
                tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
                std::cout << "\n";
            }
            
            if (ssh_userauth_kbdint_setanswer(sshSession.get(), iprompt, answer.c_str()) < 0) {
                return false;
            }
        }
        rc = ssh_userauth_kbdint(sshSession.get(), nullptr, nullptr);
    }
    
    return (rc == SSH_AUTH_SUCCESS);
}

bool SshHandler::upload(const std::string path)
{
    int rc, nwritten;
    if (!sshConnect()) return false;

    sftp_session sftp = sftp_new(sshSession.get());
    if (sftp == NULL) {
        std::cerr << "Error allocating SFTP session: " << ssh_get_error(sshSession.get()) << "\n";
        return false;
    }
  
    rc = sftp_init(sftp);
    if (rc != SSH_OK) {
        std::cerr << "Error initializing SFTP session: " << sftp_get_error(sftp) << "\n";
        sftp_free(sftp);
        return false;
    }
    
    sftp_file file = sftp_open(sftp, path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR);
    if (file == NULL) {
        std::cerr << "Can't open remote file for writing: " << ssh_get_error(sshSession.get()) << "\n";
        sftp_free(sftp);
        return false;
    }

    std::ifstream fin(path, std::ios::binary);
    if (!fin.is_open()) {
        std::cerr << "Can't open local file: " << path << "\n";
        sftp_close(file);
        sftp_free(sftp);
        return false;
    }

    constexpr size_t max_xfer_buf_size = 10240;
    char buffer[max_xfer_buf_size];
    bool success = true;

    while (fin.read(buffer, sizeof(buffer)) || fin.gcount() > 0) {
        nwritten = sftp_write(file, buffer, fin.gcount());
        if (nwritten != fin.gcount()) {
            std::cerr << "Error writing to remote file: " << ssh_get_error(sshSession.get()) << "\n";
            success = false;
            break;
        }
    }

    sftp_close(file);
    sftp_free(sftp);
    return success;
}

bool SshHandler::serviceUpload(const std::string path)
{
    int rc, nwritten;
    if (!sshConnect()) return false;

    sftp_session sftp = sftp_new(sshSession.get());
    if (sftp == NULL) {
        std::cerr << "Error allocating SFTP session: " << ssh_get_error(sshSession.get()) << "\n";
        return false;
    }
  
    rc = sftp_init(sftp);
    if (rc != SSH_OK) {
        std::cerr << "Error initializing SFTP session\n";
        sftp_free(sftp);
        return false;
    }

    if (path.find('/') != std::string::npos) {
        std::string cpy_path = path;
        std::string dir = "";
        while (cpy_path.find('/') != std::string::npos) {
            dir += cpy_path.substr(0, cpy_path.find_first_of('/') + 1);
            cpy_path = cpy_path.substr(cpy_path.find_first_of('/') + 1);
            rc = sftp_mkdir(sftp, dir.c_str(), S_IRWXU);
            if (rc != SSH_OK && sftp_get_error(sftp) != SSH_FX_FILE_ALREADY_EXISTS) {
                std::cerr << "Can't create directory: " << dir << "\n";
                sftp_free(sftp);
                return false;
            }
        }
    }
    
    sftp_file file = sftp_open(sftp, path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR);
    if (file == NULL) {
        std::cerr << "Can't open remote file: " << ssh_get_error(sshSession.get()) << "\n";
        sftp_free(sftp);
        return false;
    }

    std::ifstream fin(path, std::ios::binary);
    if (!fin.is_open()) {
        sftp_close(file);
        sftp_free(sftp);
        return false;
    }

    constexpr size_t max_xfer_buf_size = 10240;
    char buffer[max_xfer_buf_size];
    bool success = true;

    while (fin.read(buffer, sizeof(buffer)) || fin.gcount() > 0) {
        nwritten = sftp_write(file, buffer, fin.gcount());
        if (nwritten != fin.gcount()) {
            std::cerr << "Error writing data.\n";
            success = false;
            break;
        }
    }

    sftp_close(file);
    sftp_free(sftp);
    return success;
}

SshHandler::SftpSessionPtr SshHandler::initSftp() 
{
    if (!sshConnect()) return nullptr;

    sftp_session raw_sftp = sftp_new(sshSession.get());
    if (!raw_sftp) return nullptr;

    SftpSessionPtr sftp(raw_sftp);
    if (sftp_init(sftp.get()) != SSH_OK) return nullptr;

    return sftp;
}

bool SshHandler::inject_local_public_key(const std::string& key_path) {
    std::string pubkey_content;
    std::string source_path = key_path;

    if (source_path.empty()) {
        const char* home_dir = std::getenv("HOME");
        if (!home_dir) return false;
        source_path = std::string(home_dir) + "/.ssh/id_rsa.pub";
    }

    std::ifstream pubkey_file(source_path);
    if (!pubkey_file.is_open()) {
        return false;
    }
    
    std::getline(pubkey_file, pubkey_content);
    while (!pubkey_content.empty() && 
           (pubkey_content.back() == '\r' || pubkey_content.back() == '\n' || pubkey_content.back() == ' ')) {
        pubkey_content.pop_back();
    }

    if (pubkey_content.empty()) return false;

    std::string remote_cmd = 
        "mkdir -p ~/.ssh && "
        "chmod 700 ~/.ssh && "
        "touch ~/.ssh/authorized_keys && "
        "chmod 600 ~/.ssh/authorized_keys && "
        "if ! grep -qF '" + pubkey_content + "' ~/.ssh/authorized_keys; then "
        "  echo '" + pubkey_content + "' >> ~/.ssh/authorized_keys; "
        "fi";
    
    return exec_remote_command(remote_cmd);
}

bool SshHandler::attemptConnection() {
    if (!sshConnect()) return false;

    int rc = ssh_userauth_none(sshSession.get(), nullptr);
    if (rc == SSH_AUTH_SUCCESS) {
        authenticated = true;
        return true;
    }

    int methods = ssh_userauth_list(sshSession.get(), nullptr);

    if (methods & SSH_AUTH_METHOD_PUBLICKEY) {
        rc = ssh_userauth_publickey_auto(sshSession.get(), nullptr, nullptr);
        if (rc == SSH_AUTH_SUCCESS) {
            authenticated = true;
            return true;
        }
    }

    if (methods & SSH_AUTH_METHOD_INTERACTIVE) {
        if (authenticate_kbdint()) {
            authenticated = true;
            return true;
        }
    }

    return false;
}

bool SshHandler::connect() {
    if (!sshConnect()) return false;
    int rc = ssh_userauth_publickey_auto(sshSession.get(), nullptr, nullptr);
    if (rc == SSH_AUTH_SUCCESS) {
        authenticated = true;
        return true;
    }
    return false;
}

bool SshHandler::upload_to_dest(const std::string& local_path, const std::string& remote_path)
{
    int rc, nwritten;
    if (!sshConnect()) return false;

    auto sftp = initSftp();
    if (!sftp) return false;

    if (remote_path.find('/') != std::string::npos) {
        std::string dir = "";
        std::string cpy_path = remote_path;
        while (cpy_path.find('/') != std::string::npos) {
            size_t pos = cpy_path.find_first_of('/');
            dir += cpy_path.substr(0, pos + 1);
            cpy_path = cpy_path.substr(pos + 1);
            
            if (dir == "/" || dir.empty()) continue;

            rc = sftp_mkdir(sftp.get(), dir.c_str(), S_IRWXU);
            if (rc != SSH_OK && sftp_get_error(sftp.get()) != SSH_FX_FILE_ALREADY_EXISTS) {
            }
        }
    }
    
    sftp_file file = sftp_open(sftp.get(), remote_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (file == NULL) {
        std::cerr << "Can't open remote file for writing: " << remote_path << "\n";
        return false;
    }

    std::ifstream fin(local_path, std::ios::binary);
    if (!fin.is_open()) {
        std::cerr << "Can't open local build source: " << local_path << "\n";
        sftp_close(file);
        return false;
    }

    constexpr size_t max_xfer_buf_size = 10240;
    char buffer[max_xfer_buf_size];
    bool success = true;

    while (fin.read(buffer, sizeof(buffer)) || fin.gcount() > 0) {
        nwritten = sftp_write(file, buffer, fin.gcount());
        if (nwritten != fin.gcount()) {
            std::cerr << "Error writing block segments over network layer.\n";
            success = false;
            break;
        }
    }

    sftp_close(file);
    return success;
}