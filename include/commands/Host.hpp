#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <memory>
#include <map>
#include <vector>
#include <functional>

#include "ICommand.hpp"
class SshHandler;

inline constexpr std::string_view HOST_HELP = R"(
    Usage: host <action> [arguments...]
        Manages host configurations.
        host setup [--skip-nginx] [--caffeine-version <string>]: Remotely provisions a clean OS into a fully functioning Tiramisu node.
        host reset [--keep-db] [-y, --yes]: Wipes out all deployed application functions while leaving the underlying Nginx, Caffeine, and database configurations intact.
        host purge [-y, --yes]: The destructive deep-clean. Uninstalls Nginx, deletes the Caffeine binary, wipes all systemd services, and flushes all storage folders.
        host add <env_name> [--ip <string>] [--user <string>] [--password <string>] [--port <int>]: add a new host to the hosts list
        host ls list: list stored hosts
        host test <env_name>: test connection with the specified host
)";

class Host : public ICommand {
    private:
    std::string getArch(const Command&& command) const;
    // factory method
    void add(const Command&& command);
    void list(const Command&& command);
    void setup(const Command&& command);
    void reset(const Command&& command);
    void purge(const Command&& command);
    void test(const Command&& command);

    public:
    Host() = default;
    ~Host() override {};

    std::string getName() const override;
    std::string_view getHelp() const override;
    void execute(const Command& command) override;

    std::map<std::string, std::function<void(const Command&& command)>> commandsMap = {
        {"add",   [this](const Command&& cmd) { add(std::move(cmd)); }},
        {"list",  [this](const Command&& cmd) { list(std::move(cmd)); }},
        {"ls",    [this](const Command&& cmd) { list(std::move(cmd)); }},
        {"setup", [this](const Command&& cmd) { setup(std::move(cmd)); }},
        {"reset", [this](const Command&& cmd) { reset(std::move(cmd)); }},
        {"purge", [this](const Command&& cmd) { purge(std::move(cmd)); }},
        {"test",  [this](const Command&& cmd) { test(std::move(cmd)); }},
    };
};
