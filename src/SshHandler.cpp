#include "../include/SshHandler.hpp"

#include <ios>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <cstring>

SshHandler::SshHandler(const std::string& host, const std::string& user, int port)
{
    sshSession = SshSessionPtr(ssh_new());
    if (sshSession) {
        ssh_options_set(sshSession.get(), SSH_OPTIONS_HOST, host.c_str());
        ssh_options_set(sshSession.get(), SSH_OPTIONS_PORT, &port);
        ssh_options_set(sshSession.get(), SSH_OPTIONS_USER, user.c_str());
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
    ssh_channel channel;
    int rc;

    if (!sshConnect()) {
        const char *err = ssh_get_error(sshSession.get());
        throw std::runtime_error(err);
    }
    channel = ssh_channel_new(sshSession.get());

    if (channel == NULL) {
        const char *err = ssh_get_error(sshSession.get());
        throw std::runtime_error(err);
    }
    
    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        ssh_channel_free(channel);
        const char *err = ssh_get_error(sshSession.get());
        throw std::runtime_error(err);
    }
    return channel;
}

std::string SshHandler::getArch()
{
    if (!sshConnect()) return "";

    ssh_channel channel = ssh_channel_new(sshSession.get());
    if (!channel || ssh_channel_open_session(channel) != SSH_OK) return "";

    std::string arch = "";
    if (ssh_channel_request_exec(channel, "uname -m") == SSH_OK) {
        char buffer[256];
        int nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
        while (nbytes > 0) {
            arch.append(buffer, nbytes);
            nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
        }
    }
    
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    // Clean up trailing newline characters common with unix console returns
    arch.erase(arch.find_last_not_of(" \n\r\t") + 1);
    return arch;
}

int SshHandler::exec_remote_command(const std::string& command)
{
    ssh_channel channel;
    int rc;
    char buffer[256];
    int nbytes;
    
    channel = initChannel();
    
    rc = ssh_channel_request_exec(channel, command.c_str());
    if (rc != SSH_OK) {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return rc;
    }
    
    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    while (nbytes > 0) {
        if (write(1, buffer, nbytes) != (unsigned int) nbytes) {
            ssh_channel_close(channel);
            ssh_channel_free(channel);
            return SSH_ERROR;
        }
        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    }
    
    if (nbytes < 0) {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return SSH_ERROR;
    }
  
    CLOSE_CHANNEL(channel);
  
    return SSH_OK;
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
            std::cerr << "Host key for server changed: it is now:\n";
            std::cerr << "Public key hash" << std::hex << hash << hlen;
            std::cerr <<  "For security reasons, connection will be stopped\n";
            ssh_clean_pubkey_hash(&hash);
            return false;
        case SSH_KNOWN_HOSTS_OTHER:
            std::cerr << "The host key for this server was not found but an other type of key exists.\n";
            std::   cerr << "An attacker might change the default server key to confuse your client into thinking the key does not exist\n";
            ssh_clean_pubkey_hash(&hash);
            return false;
        case SSH_KNOWN_HOSTS_NOT_FOUND:
            std::cerr << "Could not find known host file.\n";
            std::cerr << "If you accept the host key here, the file will be automatically created.\n";
            [[fallthrough]];
        case SSH_KNOWN_HOSTS_UNKNOWN:
            hexa = ssh_get_hexa(hash, hlen);
            std::cerr << "The server is unknown. Do you trust the host key?\n";
            std::cerr << "Public key hash: %s\n" << hexa;
            ssh_string_free_char(hexa);
            ssh_clean_pubkey_hash(&hash);
            std::cin >> buf;
            if (buf.empty() || (buf != "yes")) {
                return false;
            }
 
            rc = ssh_session_update_known_hosts(sshSession.get());
            if (rc < 0) {
                std::cerr << "Error " << errno << "\n";
                return false;
            }
            break;
        case SSH_KNOWN_HOSTS_ERROR:
            std::cerr << "Error " << ssh_get_error(sshSession.get());
            ssh_clean_pubkey_hash(&hash);
            return false;
    }
    ssh_clean_pubkey_hash(&hash);
    return true;
}

bool SshHandler::connect_with_password(const std::string& password) 
{
    int rc = ssh_userauth_password(sshSession.get(), nullptr, password.c_str());
    if (rc == SSH_AUTH_SUCCESS) return true;

    if (rc == SSH_AUTH_DENIED || rc == SSH_AUTH_PARTIAL) {
        rc = ssh_userauth_kbdint(sshSession.get(), nullptr, nullptr);
        
        while (rc == SSH_AUTH_INFO) {
            int nprompts = ssh_userauth_kbdint_getnprompts(sshSession.get());
            
            for (int i = 0; i < nprompts; ++i) {
                const char* prompt = ssh_userauth_kbdint_getprompt(sshSession.get(), i, nullptr);
                std::string prompt_str(prompt);
                
                if (prompt_str.find("password") != std::string::npos || 
                    prompt_str.find("Password") != std::string::npos) {
                    ssh_userauth_kbdint_setanswer(sshSession.get(), i, password.c_str());
                }
            }
            rc = ssh_userauth_kbdint(sshSession.get(), nullptr, nullptr);
        }
    }
    return (rc == SSH_AUTH_SUCCESS);
}

int SshHandler::authenticate_kbdint()
{
  int rc;
 
  rc = ssh_userauth_kbdint(sshSession.get(), NULL, NULL);
  while (rc == SSH_AUTH_INFO)
  {
    const char *name = NULL, *instruction = NULL;
    int nprompts, iprompt;
 
    name = ssh_userauth_kbdint_getname(sshSession.get());
    instruction = ssh_userauth_kbdint_getinstruction(sshSession.get());
    nprompts = ssh_userauth_kbdint_getnprompts(sshSession.get());
 
    if (std::strlen(name) > 0)
      std::cout << name << "\n";
    if (std::strlen(instruction) > 0)
      std::cout << instruction << "\n";
    for (iprompt = 0; iprompt < nprompts; iprompt++)
    {
      const char *prompt = NULL;
      char echo;
 
      prompt = ssh_userauth_kbdint_getprompt(sshSession.get(), iprompt, &echo);
      if (echo)
      {
        char buffer[128], *ptr;
 
        std::cout << prompt << "\n";
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
          return SSH_AUTH_ERROR;
        buffer[sizeof(buffer) - 1] = '\0';
        if ((ptr = std::strchr(buffer, '\n')) != NULL)
          *ptr = '\0';
        if (ssh_userauth_kbdint_setanswer(sshSession.get(), iprompt, buffer) < 0)
          return SSH_AUTH_ERROR;
        memset(buffer, 0, strlen(buffer));
      }
      else
      {
        char *ptr = NULL;
 
        ptr = getpass(prompt);
        if (ssh_userauth_kbdint_setanswer(sshSession.get(), iprompt, ptr) < 0)
          return SSH_AUTH_ERROR;
      }
    }
    rc = ssh_userauth_kbdint(sshSession.get(), NULL, NULL);
  }
  return rc;
}

void SshHandler::upload(const std::string path)
{
    std::cout << path << "\n";
    int rc, nwritten;
    
    sftp_session sftp;
    sshConnect();
    sftp = sftp_new(sshSession.get());
    if (sftp == NULL)
    {
      fprintf(stderr, "Error allocating SFTP session: %s\n", ssh_get_error(sshSession.get()));
      exit(1);
    }
  
    rc = sftp_init(sftp);
    if (rc != SSH_OK)
    {
      fprintf(stderr, "Error initializing SFTP session: code %d.\n", sftp_get_error(sftp));
      sftp_free(sftp);
      exit(1);
    }
    
    sftp_file file = sftp_open(sftp, path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR);
    if (file == NULL)
    {
      fprintf(stderr, "Can't open file for writing: %s\n", ssh_get_error(sshSession.get()));
      exit(1);
    }

    std::ifstream fin(path, std::ios::binary);

    constexpr size_t max_xfer_buf_size = 10240;
    while (fin)
    {
        char buffer[max_xfer_buf_size];
        fin.read(buffer, sizeof(buffer));
        if (fin.gcount() > 0)
        {
            nwritten = sftp_write(file, buffer, fin.gcount());
            if (nwritten != fin.gcount())
            {
                fprintf(stderr, "Error writing to file: %s\n", ssh_get_error(sshSession.get()));
                sftp_close(file);
                exit(1);
            }
        }
    }

    rc = sftp_close(file);
    if (rc != SSH_OK)
    {
      fprintf(stderr, "Can't close the written file: %s\n", ssh_get_error(sshSession.get()));
      exit(1);
    }

    sftp_free(sftp);
}

void SshHandler::serviceUpload(const std::string path)
{
    std::cout << path << "\n";
    int rc, nwritten;
    
    sftp_session sftp;
    sshConnect();
    sftp = sftp_new(sshSession.get());
    if (sftp == NULL)
    {
      fprintf(stderr, "Error allocating SFTP session: %s\n", ssh_get_error(sshSession.get()));
      exit(1);
    }
  
    rc = sftp_init(sftp);
    if (rc != SSH_OK)
    {
      fprintf(stderr, "Error initializing SFTP session: code %d.\n", sftp_get_error(sftp));
      sftp_free(sftp);
      exit(1);
    }

    if (path.find('/') != std::string::npos) {
        std::string cpy_path = path;
        std::string dir = "";
        while (cpy_path.find('/') != std::string::npos) {
            dir += cpy_path.substr(0, cpy_path.find_first_of('/') + 1);
            std::cout << "dir: " << dir << "\n";
            cpy_path = cpy_path.substr(cpy_path.find_first_of('/') + 1);
            rc = sftp_mkdir(sftp, dir.c_str(), S_IRWXU);
            if (rc != SSH_OK) {
                if (sftp_get_error(sftp) != SSH_FX_FILE_ALREADY_EXISTS) {
                fprintf(stderr, "Can't create directory: %s\n", ssh_get_error(sshSession.get()));
                    exit(1);
                }
            }
        }
    }
    
    std::cout << "path: " + path << std::endl;
    sftp_file file = sftp_open(sftp, path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR);
    if (file == NULL)
    {
      fprintf(stderr, "Can't open file for writing: %s\n", ssh_get_error(sshSession.get()));
      exit(1);
    }

    std::ifstream fin(path, std::ios::binary);

    constexpr size_t max_xfer_buf_size = 10240;
    while (fin)
    {
        char buffer[max_xfer_buf_size];
        fin.read(buffer, sizeof(buffer));
        if (fin.gcount() > 0)
        {
            nwritten = sftp_write(file, buffer, fin.gcount());
            if (nwritten != fin.gcount())
            {
                fprintf(stderr, "Error writing to file: %s\n", ssh_get_error(sshSession.get()));
                sftp_close(file);
                exit(1);
            }
        }
    }

    rc = sftp_close(file);
    if (rc != SSH_OK)
    {
      fprintf(stderr, "Can't close the written file: %s\n", ssh_get_error(sshSession.get()));
      exit(1);
    }

    sftp_free(sftp);
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
