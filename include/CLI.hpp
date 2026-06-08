#pragma once

#include <Parser.hpp>
#include <commands/ICommand.hpp>

#include <functional> // std::function
#include <stdlib.h>
#include <memory>
#include <map>

class Webserver;
class Parser;
class SshHandler;

class CLI
{
    private:
        std::unique_ptr<Parser> m_parser;
        std::map<std::string, std::string> m_env;
        std::unique_ptr<SshHandler> m_sshHandler;
        
        void loadEnvFile(const std::string& file_path);
        void processParsedCommand(const Command& command);
        void displayHelp(const std::string& command_name = "");

        void executeConnect(const std::string& host, const std::string& user, const std::string& password);
        void executeSSH(const std::string& Commando_execute);
        void executeSetEnv(const std::string& key, const std::string& value);
        bool create_config_dir();

        // factory method
        std::vector<std::function<std::unique_ptr<ICommand>()>> m_commandFactories;
        void registerCommandFactories();

    public:
        CLI(std::unique_ptr<Parser> parser, const std::string& env_path);
        ~CLI();

        void run();
};
