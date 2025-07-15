#pragma once

#include "ICommand.hpp"

class Install : public ICommand {
    public:
    Install();
    ~Install() override {};
    std::string getName() const override;
    std::string getHelp() const override;
    void execute(const Command_t& command) override;
};