#pragma once

#include "ICommand.hpp"
#include <map>

class Setup : public ICommand {
    private:

    public:
    Setup();
    ~Setup();
    std::string getName() const override;
    std::string getHelp() const override;
    void execute(const Command_t& command) override;
};