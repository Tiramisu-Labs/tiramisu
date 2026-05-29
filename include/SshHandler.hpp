#pragma once

#include <string>
#define LIBSSH_STATIC 1
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <vector>
#include <memory>
#include <map>

#define CLOSE_CHANNEL(channel) ssh_channel_send_eof(channel); ssh_channel_close(channel); ssh_channel_free(channel)

struct SshSessionDeleter {
    void operator()(ssh_session session) const {
        if (session) {
            ssh_disconnect(session); 
            ssh_free(session);
        }
    }
};

class SshHandler
{
    private:
        std::unique_ptr<typename std::remove_pointer<ssh_session>::type, SshSessionDeleter> sshSession;
        std::string m_host      = "";
        std::string m_user      = "";
        int m_port              = 22;
        std::string m_password  = "";

    public:
        SshHandler() = default;
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
        bool sshConnect(const std::string& host, const std::string& user, int port, const char* password = nullptr);
        void sshDisconnect();

        bool verify_knownhost(ssh_session session);
        int exec_remote_command(const std::string& command);

        void upload(const std::string path);
        void serviceUpload(const std::string path);

        bool isConnected() { return sshSession != nullptr; }
};
