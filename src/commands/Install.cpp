#include "../../include/commands/Install.hpp"

#include <stdexcept>
#include <iostream>

Install::Install() {}

std::string Install::getName() const { return "install"; }
std::string Install::getHelp() const {
    return "Try to install the desired package:\n"
        "  emscripten   install emscripten to compile C and C++ into wasm";
}

void Install::execute(const Command_t& command) {
    std::cout << "execute install\n";
    if (command.arguments.size() == 0) {
        throw std::runtime_error("no arguments provided");
    }
    int status = 0;
    const std::string package = command.arguments.front();
    std::cout << package << "\n";
    if (package == "emscripten") {
        status = system(
            "git clone https://github.com/emscripten-core/emsdk.git $HOME/emsdk && "
            "cd $HOME/emsdk && git pull && "
            "./emsdk install latest && "
            "./emsdk activate latest && "
            "source $HOME/emsdk/emsdk_env.sh && "
            "echo 'source $HOME/emsdk/emsdk_env.sh' >> $HOME/.bash_profile"
        );
        if (!status) {
            throw std::runtime_error("something went wrong");
        }
        std::cout << "emsdk added at location " << std::getenv("HOME") <<
            "/ and source $HOME/emsdk/emsdk_env.sh added to " <<
            std::getenv("HOME") << "/.bash_profile\n";
    }
}