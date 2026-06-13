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

struct SftpSessionDeleter {
    void operator()(sftp_session session) const {
        if (session) {
            sftp_free(session);
        }
    }
};

class SshHandler
{
    private:

        using SshSessionPtr  = std::unique_ptr<ssh_session_struct, SshSessionDeleter>;
        using SftpSessionPtr = std::unique_ptr<sftp_session_struct, SftpSessionDeleter>;

        SshSessionPtr sshSession;
        bool connected = false;
        bool authenticated = false;

        SftpSessionPtr initSftp();
        ssh_channel initChannel();

        bool sshConnect();
        bool authenticate_kbdint();
        bool verify_knownhost();
    public:
        SshHandler() = delete;
        SshHandler(const std::string& host, const std::string& user, int port, const std::string& key_path = "");
        SshHandler& operator=(const SshHandler&) = delete;

        ~SshHandler() = default;
    
        std::string getArch();
        
        bool attemptConnection();
        bool connect();
        bool try_password(const std::string& password); // REMOVE IN FINAL PRODUCTION

        bool exec_remote_command(const std::string& command);

        bool upload(const std::string path);
        bool serviceUpload(const std::string path);
        bool upload_to_dest(const std::string& local_path, const std::string& remote_path);

        bool isConnected() const { return sshSession != nullptr; }
        bool inject_local_public_key(const std::string& custom_pubkey_path);
};
