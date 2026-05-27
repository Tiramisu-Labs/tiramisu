#include <commands/Create.hpp>

#include <iostream>

Create::Create() {};

std::string Create::getName() const { return "create"; }

std::string_view Create::getHelp() const {
    return CREATE_HELP;
}

void Create::execute(const Command& command) {
    std::cout << "execute create\n";
    
    if (command.help) {
        std::cout << getHelp();
        return;
    }
    if (command.arguments.size() == 0 || command.options.size() == 0) {
        throw std::runtime_error(std::string(getHelp()));
    }

    const auto options = command.options;    
    const auto routh_path = command.arguments.front();

}
