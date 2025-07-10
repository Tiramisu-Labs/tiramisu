#include "../include/Webserver.hpp"
#include "../include/Utils.hpp"
#include "../include/SshHandler.hpp"
#include "../include/Token.hpp"

#include <iostream>
#include <algorithm>
#include <fstream>
#include <map>

Webserver::Webserver() : handler(std::make_unique<SshHandler>()) {}
Webserver::~Webserver() {}

void Webserver::exec(std::string address, std::string cmd, std::vector<Token> flags, int port)
{
    static std::map<std::string, EWebCommands> commandsMap = {
        {"login", EWebCommands::LOGIN},
        {"start", EWebCommands::START},
        {"run", EWebCommands::RUN},
        {"reload", EWebCommands::RELOAD},
        {"config", EWebCommands::CONFIG}
    };
    switch (commandsMap[cmd])
    {
    case EWebCommands::UNRECOGNIZED:
        std::cout << "case unrecognized\n";
        exit(1);
    case EWebCommands::LOGIN:
    {
        std::cout << "case login\n";
        auto password_it = std::find_if(
            flags.begin(),
            flags.end(),
            [] (Token & t) { return t.getValue() == "--password"; }
        );
        std::string password = "";
        if (password_it != flags.end()) {
            password = password_it->getValue();
        }
        std::cout << "password " + password + " address " + address << "\n";
        handler->sshConnection(address, password, port);
        break;
    }
    case EWebCommands::START:
        std::cout << "case start\n";
        break;
    case EWebCommands::RUN:
    {
        std::cout << "case run\n";
        std::ifstream file("config/config.yaml");
        std::string line;
        while(std::getline(file, line)) {
            std::cout << line << "\n";
        }
        break;
    }
    case EWebCommands::RELOAD:
        std::cout << "case reload\n";
        break;
    case EWebCommands::CONFIG:
        std::cout << "case config\n";
        break;
    default:
        std::cout << "case default\n";
        return ;
        break;
    }
}

void Webserver::upload(const std::string address, const std::string password, int port, std::string path)
{
    handler->sshConnection(address, password, port);
    handler->upload(path);
}