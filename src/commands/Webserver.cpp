#include "../../include/commands/Webserver.hpp"

#include "../../include/SshHandler.hpp"
#include <iostream>

Webserver::Webserver() {}
Webserver::Webserver(std::unique_ptr<SshHandler>&& handler) : m_sshHandler(std::move(handler)) {}

std::string Webserver::getName() const { return "webserver"; }

std::string Webserver::getHelp() const {
    return "";
}

void Webserver::execute(const Command_t& command) {
    std::cout << command.name << std::endl;

    switch (commandsMap[command.name])
    {
    case webserver::Commands::INVALID:
        std::cout << "case unrecognized\n";
        exit(1);
    // case webserver::Commands::LOGIN:
    // {
    //     std::cout << "case login\n";
    //     auto password_it = std::find_if(
    //         flags.begin(),
    //         flags.end(),
    //         [] (Token & t) { return t.getValue() == "--password"; }
    //     );
    //     std::string password = "";
    //     if (password_it != flags.end()) {
    //         password = password_it->getValue();
    //     }
    //     std::cout << "password " + password + " address " + address << "\n";
    //     handler->sshConnection(address, password, port);
    //     break;
    // }
    case webserver::Commands::START:
        std::cout << "case start\n";
        break;
    // case webserver::Commands::RUN:
    // {
    //     std::cout << "case run\n";
    //     std::ifstream file("config/config.yaml");
    //     std::string line;
    //     while(std::getline(file, line)) {
    //         std::cout << line << "\n";
    //     }
    //     break;
    // }
    case webserver::Commands::RELOAD:
        std::cout << "case reload\n";
        break;
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