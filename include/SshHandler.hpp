#pragma once

#include <string>
#define LIBSSH_STATIC 1
#include <libssh/libssh.h>
#include <libssh/sftp.h>

class SshHandler
{
    private:
        ssh_session m_session;
        std::string m_host;
        std::string m_user;
        int m_port;
        std::string m_password;

    public:
        SshHandler();
        ~SshHandler();
    
        void setHost(std::string&& host);
        void setUser(std::string&& user);
        void setPort(int port);
        void setPassword(std::string&& password);
        
        bool connect(const std::string& host, const std::string user, int port, const std::string password);
        bool sshConnection(const std::string & url, int port);
        bool sshConnection(const std::string & url, const std::string & password, int port);
        bool sshConnection(const std::string& host, const std::string& user, int port, const std::string password);

        bool verify_knownhost(ssh_session session);
        int exec_remote_command(std::string command);

        void upload(const std::string path);

        bool isConnected() { return m_session != nullptr; }
};
