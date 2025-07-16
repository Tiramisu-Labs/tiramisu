#include "../../include/commands/Webserver.hpp"

#include "../../include/SshHandler.hpp"
#include <iostream>
#include "Webserver.hpp"

Webserver::Webserver() {}
Webserver::Webserver(std::unique_ptr<SshHandler>&& handler) : m_sshHandler(std::move(handler)) {}

std::string Webserver::getName() const { return "webserver"; }

std::string Webserver::getHelp() const {
    return "";
}

void Webserver::execute(const Command_t& command) {
    std::cout << command.arguments.front() << std::endl;

    switch (commandsMap[command.arguments.front()])
    {
    case webserver::Commands::INVALID:
        std::cout << "case unrecognized\n";
        exit(1);
    case webserver::Commands::START: { // start remote server
        const auto alias_it = command.options.find("--alias");
        if (alias_it == command.options.end()) throw std::runtime_error("alias option is missing!");
        m_sshHandler->fillSshHandler(alias_it->second);
        m_sshHandler->exec_remote_command("$HOME/nginx/sbin/nginx");
        break;
    }
    case webserver::Commands::STOP: { // stop remote server
        const auto alias_it = command.options.find("--alias");
        if (alias_it == command.options.end()) throw std::runtime_error("alias option is missing!");
        m_sshHandler->fillSshHandler(alias_it->second);
        m_sshHandler->exec_remote_command("$HOME/nginx/sbin/nginx -s stop");
        break;
    }
    case webserver::Commands::RESTART: { // restart remote server
        const auto alias_it = command.options.find("--alias");
        if (alias_it == command.options.end()) throw std::runtime_error("alias option is missing!");
        m_sshHandler->fillSshHandler(alias_it->second);
        std::cout << "stopping nginx...\n";
        m_sshHandler->exec_remote_command("$HOME/nginx/sbin/nginx -s stop");
        std::cout << "restarting nginx...\n";
        m_sshHandler->exec_remote_command("$HOME/nginx/sbin/nginx");
        break;
    }
    case webserver::Commands::DEPLOY: { // upload passed folder to remote host and setup nginx to work with it

    }
    case webserver::Commands::CONFIGURE:
        std::cout << "case config\n";
        break;
    default:
        std::cout << "case default\n";
        return ;
        break;
    }
}

void Webserver::upload(std::string host, std::string password, std::string user, std::string port, std::string path)
{
    m_sshHandler->fillSshHandler(host, password, user, port);
    m_sshHandler->sshConnect();
    m_sshHandler->upload(path);
    m_sshHandler->sshDisconnect();
}
void Webserver::deploy(const Command_t &command)
{
    command.arguments.top();
}