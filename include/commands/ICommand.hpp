#pragma once

#include <string>

#include "../Utils.hpp"

class ICommand {
    public:
        virtual ~ICommand() = default;
        virtual std::string getName() const = 0;
        virtual void execute(const Command_t& command) = 0;
        virtual std::string getHelp() const = 0;
};