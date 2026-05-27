#include <commands/Serve.hpp>

#include <iostream>

std::string Serve::getName() const { return "serve"; };

std::string_view Serve::getHelp() const { return BUILD_SERVE; }

void Serve::execute(const Command& command)
{
    if (command.help) {
        std::cout << getHelp();
        return ;
    }

    const auto path = command.arguments.front();

    // upload all folder where nginx search for static content
}
