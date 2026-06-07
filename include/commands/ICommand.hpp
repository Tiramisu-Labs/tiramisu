#pragma once

#include <string>

#include <Utils.hpp>
#include <string_view>

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
    LOCAL,
    SIZE
};



class ICommand {
    public:
        virtual ~ICommand() = default;
        virtual std::string getName() const = 0;
        virtual void execute(const Command& command) = 0;
        virtual std::string_view getHelp() const = 0;
};
