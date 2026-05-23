#include <commands/Serve.hpp>

#include <iostream>

std::string Serve::getName() const { return "serve"; };

std::string_view Serve::getHelp() const { return BUILD_SERVE; }

void Serve::execute(const Comman_t& command)
{
    if (command.help) {
        std::cout << getHelp();
        return ;
    }

    const auto path = command.argument.front();

    // upload all folder where nginx search for static content
}
