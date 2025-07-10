#include "../../include/commands/Setup.hpp"

#include <iostream>

Setup::Setup() {}

std::string Setup::getName() const { return "setup"; }

std::string Setup::getHelp() const {
    return "Usage: setup <host> [arguments...]\n"
        "  install packages inside the remote host.\n"
        "  Default: nginx";
}

void Setup::execute(const Command_t & command) {
    
}