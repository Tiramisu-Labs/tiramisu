#include <commands/Webserver.hpp>
#include <SshHandler.hpp>
#include <iostream>

std::string Webserver::getName() const { return "webserver"; }
std::string_view Webserver::getHelp() const {
    return "";
}

void Webserver::execute(const Command& command) {
    std::cout << command.arguments.front() << std::endl;

    const auto handler = std::make_unique<SshHandler>();
    switch (commandsMap[command.arguments.front()])
    {
    case webserver::Commands::INVALID:
        std::cout << "case unrecognized\n";
        exit(1);
    case webserver::Commands::START: { // start remote server
        const auto alias_it = command.options.find("--alias");
        if (alias_it == command.options.end()) throw std::runtime_error("alias option is missing!");
        handler->fillSshHandler(alias_it->second);
        handler->exec_remote_command("$HOME/nginx/sbin/nginx");
        break;
    }
    case webserver::Commands::STOP: { // stop remote server
        const auto alias_it = command.options.find("--alias");
        if (alias_it == command.options.end()) throw std::runtime_error("alias option is missing!");
        handler->fillSshHandler(alias_it->second);
        handler->exec_remote_command("$HOME/nginx/sbin/nginx -s stop");
        break;
    }
    case webserver::Commands::RESTART: { // restart remote server
        const auto alias_it = command.options.find("--alias");
        if (alias_it == command.options.end()) throw std::runtime_error("alias option is missing!");
        handler->fillSshHandler(alias_it->second);
        std::cout << "stopping nginx...\n";
        handler->exec_remote_command("$HOME/nginx/sbin/nginx -s stop");
        std::cout << "restarting nginx...\n";
        handler->exec_remote_command("$HOME/nginx/sbin/nginx");
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
    const auto handler = std::make_unique<SshHandler>();
    
    handler->fillSshHandler(host, password, user, port);
    handler->sshConnect();
    handler->upload(path);
    handler->sshDisconnect();
}
void Webserver::deploy(const Command &command)
{
    (void)command;
    // command.arguments.back();
}
