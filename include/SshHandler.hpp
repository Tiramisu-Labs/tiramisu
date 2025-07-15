#pragma once

#include <string>
#define LIBSSH_STATIC 1
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <vector>
#include <map>
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
        void fillSshHandler(const std::string& alias);
        void fillSshHandler(std::string host, std::string password, std::string user, std::string port);
        std::map<std::string, std::string> getHostSpec(const std::string& alias);
        std::string getArch();
        
        ssh_channel initChannel();
        bool sshConnect();
        void sshDisconnect();

        bool verify_knownhost(ssh_session session);
        int exec_remote_commands(std::vector<std::string> commands);

        void upload(const std::string path);
        void serviceUpload(const std::string path);

        bool isConnected() { return m_session != nullptr; }
};
