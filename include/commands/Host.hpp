#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <memory>
#include <map>
#include <vector>

#include "ICommand.hpp"
class SshHandler;

inline constexpr std::string_view HOST_HELP = R"(
    "Usage: host <action> [arguments...]\n"
        "  Manages host configurations.\n"
        "  host setup [--skip-nginx] [--caffeine-version <string>]: Remotely provisions a clean OS into a fully functioning Tiramisu node.\n"
        "  host reset [--keep-db] [-y, --yes]: Wipes out all deployed application functions while leaving the underlying Nginx, Caffeine, and database configurations intact."
        "  host purge [-y, --yes]: The destructive deep-clean. Uninstalls Nginx, deletes the Caffeine binary, wipes all systemd services, and flushes all storage folders."
        "  host add [--host=<IP/DNS>] [--user=<user>] [--password=<password>] [--port=<port>] [--alias=<alias>]: add a new host to the hosts list"
        "  host list: list stored hosts"
        "  host test [--alias=<alias>]: test connection with the specified host";
)";

class Host : public ICommand {
    private:
    std::unique_ptr<SshHandler> m_sshHandler;

    void add(std::unordered_map<std::string, std::string> options);
    void list();
    void test(const std::unordered_map<std::string, std::string>& options);
    void test();
    std::string getArch() const;

    public:
    Host();
    Host(std::unique_ptr<SshHandler>&& handler);
    // override
    ~Host() override {};
    std::string getName() const override;
    std::string_view getHelp() const override;
    void execute(const Command_t& command) override;

    enum class Commands {
        INVALID,
        HELP,
        ADD,
        LIST,
        EXPORT,
        IMPORT,
        DELETE,
        TEST,
        SIZE
    };

    std::map<std::string, Commands> host_cmds = {
        {"add", Commands::ADD},
        {"list", Commands::LIST},
        {"ls", Commands::LIST},
        {"export", Commands::EXPORT},
        {"import", Commands::IMPORT},
        {"delete", Commands::DELETE},
        {"test", Commands::TEST},
    };

    
};
