#include "../include/SshHandler.hpp"

#include <ios>
#include <iostream>
#include <fstream>
#include <fcntl.h>

SshHandler::SshHandler() 
    : m_host(""), m_user(""), m_port(22), m_password("") {}

SshHandler::~SshHandler() {
    if (m_session) {
        ssh_disconnect(m_session);
        ssh_free(m_session);
    }
}

void SshHandler::setHost(std::string&& host) { m_host = std::move(host); }
void SshHandler::setUser(std::string&& user) { m_user = std::move(user); }
void SshHandler::setPort(int port) { m_port = port; }
void SshHandler::setPassword(std::string&& password) { m_password = std::move(password); }

ssh_channel SshHandler::initChannel() {
    ssh_channel channel;
    int rc;

    if (!sshConnection()) {
        const char *err = ssh_get_error(m_session);
        throw std::runtime_error(err);
    }
    channel = ssh_channel_new(m_session);

    if (channel == NULL) {
        const char *err = ssh_get_error(m_session);
        throw std::runtime_error(err);
    }
    
    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        ssh_channel_free(channel);
        const char *err = ssh_get_error(m_session);
        throw std::runtime_error(err);
    }
    return channel;
}

std::string SshHandler::getArch()
{
    char buffer[256];
    ssh_channel channel;
    std::string arch = "";
    int rc, nbytes;

    channel = initChannel();
    rc = ssh_channel_request_exec(channel, "arch");
    if (rc != SSH_OK) {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        const char *err = ssh_get_error(m_session);
        throw std::runtime_error(err);
    }
    
    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    while (nbytes > 0) {
        arch += buffer;
        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    }
    
    if (nbytes < 0) {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        const char *err = ssh_get_error(m_session);
        throw std::runtime_error(err);
    }
    return arch;
}

int SshHandler::exec_remote_commands(std::vector<std::string> commands)
{
    ssh_channel channel;
    int rc;
    char buffer[256];
    int nbytes;
    
    channel = initChannel();
    for (const auto& c : commands) {
        rc = ssh_channel_request_exec(channel, c.c_str());
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
    }
  
    CLOSE_CHANNEL(channel);
  
    return SSH_OK;
}

bool SshHandler::verify_knownhost(ssh_session session)
{
    enum ssh_known_hosts_e state;
    unsigned char *hash = NULL;
    ssh_key srv_pubkey = NULL;
    size_t hlen;
    std::string buf;
    char *hexa;
    int rc;
 
    rc = ssh_get_server_publickey(session, &srv_pubkey);
    if (rc < 0) {
        return false;
    }
 
    rc = ssh_get_publickey_hash(srv_pubkey, SSH_PUBLICKEY_HASH_SHA1, &hash, &hlen);
    ssh_key_free(srv_pubkey);
    if (rc < 0) {
        return false;
    }
 
    state = ssh_session_is_known_server(m_session);
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
            std::cerr << "An attacker might change the default server key to confuse your client into thinking the key does not exist\n";
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
 
            rc = ssh_session_update_known_hosts(m_session);
            if (rc < 0) {
                std::cerr << "Error " << errno << "\n";
                return false;
            }
            break;
        case SSH_KNOWN_HOSTS_ERROR:
            std::cerr << "Error " << ssh_get_error(m_session);
            ssh_clean_pubkey_hash(&hash);
            return false;
    }
    ssh_clean_pubkey_hash(&hash);
    return true;
}

int SshHandler::exec_remote_command(std::string command)
{
    ssh_channel channel;
    int rc;
    char buffer[256];
    int nbytes;

    channel = ssh_channel_new(m_session);
    if (channel == NULL) {
        const char *err = ssh_get_error(m_session);
        std::cerr << "Error: " << err << "\n";
        return SSH_ERROR;
    }
    
    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
      ssh_channel_free(channel);
      return rc;
    }
    
    rc = ssh_channel_request_exec(channel, command.c_str());
    if (rc != SSH_OK)
    {
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return rc;
    }
  
    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    while (nbytes > 0)
    {
      if (write(1, buffer, nbytes) != (unsigned int) nbytes)
      {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return SSH_ERROR;
      }
      nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    }
  
    if (nbytes < 0)
    {
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return SSH_ERROR;
    }
  
    ssh_channel_close(channel);
    ssh_channel_send_eof(channel);
    ssh_channel_free(channel);
  
    return SSH_OK;
}

bool SshHandler::sshConnection()
{
    m_session = ssh_new();
    if (!m_session) {
        std::cerr << "ssh failed\n";
        return false;
    }

    std::string url = m_user + "@" + m_host;
    std::cout << "url: " << url << std::endl;
    ssh_options_set(m_session, SSH_OPTIONS_HOST, url.c_str());
    ssh_options_set(m_session, SSH_OPTIONS_PORT, &m_port);
    
    int rc = ssh_connect(m_session);
    if (rc != SSH_OK) {
        std::cerr << "Error connecting to " << url << ": " << ssh_get_error(m_session) << "\n";
        ssh_free(m_session);
        return false;
    }

    if (!verify_knownhost(m_session))
    {
        ssh_disconnect(m_session);
        ssh_free(m_session);
        return false;
    }

    if (m_password != "") {
        rc = ssh_userauth_password(m_session, NULL, m_password.c_str());
    } else {
        rc = ssh_userauth_publickey_auto(m_session, NULL, NULL);
    }

    if (rc != SSH_AUTH_SUCCESS)
    {
      std::cerr << "Error authenticating with password: " << ssh_get_error(m_session) << "\n";
      ssh_disconnect(m_session);
      ssh_free(m_session);
      return false;
    }

    std::cout << "ssh succesfully connected\n";
    return true;
}

bool SshHandler::sshConnection(const std::string & url, int port)
{
    m_session = ssh_new();
    if (!m_session) {
        std::cerr << "ssh failed\n";
        return false;
    }

    ssh_options_set(m_session, SSH_OPTIONS_HOST, url.c_str());
    ssh_options_set(m_session, SSH_OPTIONS_PORT, &port);
    
    int rc = ssh_connect(m_session);
    if (rc != SSH_OK) {
        std::cerr << "Error connecting to " << url << ": " << ssh_get_error(m_session) << "\n";
        ssh_free(m_session);
        return false;
    }

    if (!verify_knownhost(m_session))
    {
        ssh_disconnect(m_session);
        ssh_free(m_session);
        return false;
    }

    std::cout << "ssh succesfully connected\n";
    return true;
}

bool SshHandler::sshConnection(const std::string& host, const std::string& user, int port, const std::string password)
{
    m_session = ssh_new();
    std::cout << "sshConnection\n";
    if (!m_session) {
        std::cerr << "ssh failed\n";
        return false;
    }
    auto url = user + "@" + host;
    std::cout << "url: " << url << std::endl;
    ssh_options_set(m_session, SSH_OPTIONS_HOST, url.c_str());
    ssh_options_set(m_session, SSH_OPTIONS_PORT, &port);

    int rc = ssh_connect(m_session);
    if (rc != SSH_OK) {
        std::cerr << "Error connecting to " << url << ": " << ssh_get_error(m_session) << "\n";
        ssh_free(m_session);
        return false;
    }

    if (!verify_knownhost(m_session))
    {
        ssh_disconnect(m_session);
        ssh_free(m_session);
        return false;
    }

    if (password != "") {
        rc = ssh_userauth_password(m_session, NULL, password.c_str());
    } else {
        rc = ssh_userauth_publickey_auto(m_session, NULL, NULL);
    }

    if (rc != SSH_AUTH_SUCCESS)
    {
      std::cerr << "Error authenticating with password: " << ssh_get_error(m_session) << "\n";
      ssh_disconnect(m_session);
      ssh_free(m_session);
      return false;
    }
  
    // code here
    std::cout << "ssh succesfully connected\n";
    return true;
}

bool SshHandler::sshConnection(const std::string & url, const std::string & password, int port)
{
    m_session = ssh_new();
    if (!m_session) {
        std::cerr << "ssh failed\n";
        return false;
    }

    ssh_options_set(m_session, SSH_OPTIONS_HOST, url.c_str());
    ssh_options_set(m_session, SSH_OPTIONS_PORT, &port);

    int rc = ssh_connect(m_session);
    if (rc != SSH_OK) {
        std::cerr << "Error connecting to " << url << ": " << ssh_get_error(m_session) << "\n";
        ssh_free(m_session);
        return false;
    }

    if (!verify_knownhost(m_session))
    {
        ssh_disconnect(m_session);
        ssh_free(m_session);
        return false;
    }

    if (password != "") {
        rc = ssh_userauth_password(m_session, NULL, password.c_str());
    } else {
        rc = ssh_userauth_publickey_auto(m_session, NULL, NULL);
    }

    if (rc != SSH_AUTH_SUCCESS)
    {
      std::cerr << "Error authenticating with password: " << ssh_get_error(m_session) << "\n";
      ssh_disconnect(m_session);
      ssh_free(m_session);
      return false;
    }
  
    // code here
    std::cout << "ssh succesfully connected\n";
    return true;
}

void SshHandler::upload(const std::string path)
{
    std::cout << path << "\n";
    int rc, nwritten;
    
    sftp_session sftp;
    
    sftp = sftp_new(m_session);
    if (sftp == NULL)
    {
      fprintf(stderr, "Error allocating SFTP session: %s\n", ssh_get_error(m_session));
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
                fprintf(stderr, "Can't create directory: %s\n", ssh_get_error(m_session));
                    exit(1);
                }
            }
        }
    }
    
    std::cout << "path: " + path << std::endl;
    sftp_file file = sftp_open(sftp, path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR);
    if (file == NULL)
    {
      fprintf(stderr, "Can't open file for writing: %s\n", ssh_get_error(m_session));
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
                fprintf(stderr, "Error writing to file: %s\n", ssh_get_error(m_session));
                sftp_close(file);
                exit(1);
            }
        }
    }

    rc = sftp_close(file);
    if (rc != SSH_OK)
    {
      fprintf(stderr, "Can't close the written file: %s\n", ssh_get_error(m_session));
      exit(1);
    }

    sftp_free(sftp);
}
