#pragma once

#include "ICommand.hpp"
#include <map>
#include <memory>

class SshHandler;

namespace webserver
{
    enum class Commands
    {
        INVALID,
        INSTALL,
        START,
        RESTART,
        RELOAD,
        SHUTDOWN,
        LOGS,
        STATUS,
        CONFIGURE,
        UNISTALL,
        TEST,
        SIZE
    };
}; // namespace webserver


class Webserver : public ICommand {
    private:
    std::unique_ptr<SshHandler> m_sshHandler;

    std::map<std::string, webserver::Commands> commandsMap = {
        {"install", webserver::Commands::INSTALL},
        {"start", webserver::Commands::START},
        {"restart", webserver::Commands::RESTART},
        {"reload", webserver::Commands::RELOAD},
        {"shutdown", webserver::Commands::SHUTDOWN},
        {"logs", webserver::Commands::LOGS},
        {"status", webserver::Commands::STATUS},
        {"configure", webserver::Commands::CONFIGURE},
        {"unistall", webserver::Commands::UNISTALL},
        {"test", webserver::Commands::TEST},
    };

    public:
    Webserver();
    Webserver(std::unique_ptr<SshHandler>&& handler);
    ~Webserver() override {};
    std::string getName() const override;
    std::string getHelp() const override;
    void execute(const Command_t& command) override;
    void upload(const std::string address, const std::string password, int port, const std::string path);
};