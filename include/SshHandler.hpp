#pragma once

#include <string>
#define LIBSSH_STATIC 1
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <vector>

#define CLOSE_CHANNEL(channel) ssh_channel_send_eof(channel); ssh_channel_close(channel); ssh_channel_free(channel)

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
        void setPassword(std::string&& password);
        void setPort(int port);
        std::string getArch();
        
        ssh_channel initChannel();
        bool connect(const std::string& host, const std::string user, int port, const std::string password);
        bool sshConnection();
        bool sshConnection(const std::string & url, int port);
        bool sshConnection(const std::string & url, const std::string & password, int port);
        bool sshConnection(const std::string& host, const std::string& user, int port, const std::string password);

        bool verify_knownhost(ssh_session session);
        int exec_remote_commands(std::vector<std::string> commands);
        int exec_remote_command(std::string command);

        void upload(const std::string path);

        bool isConnected() { return m_session != nullptr; }
};
