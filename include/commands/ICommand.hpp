#pragma once

#include <string>

#include <Utils.hpp>

enum class Commands
{
    UNKNOWN,
    CONNECT,
    INIT,
    HOST,
    CREATE,
    BUILD,
    DEPLOY,
    WEBSERVER,
    INSTALL,
    SETUP,
    HELP,
    UNISTALL,
    SIZE
};



class ICommand {
    public:
        virtual ~ICommand() = default;
        virtual std::string getName() const = 0;
        virtual void execute(const Command_t& command) = 0;
        virtual std::string getHelp() const = 0;
};