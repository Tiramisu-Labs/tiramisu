#pragma once

#include "ICommand.hpp"

class Setup : public ICommand {
    private:

    public:
    Setup();
    ~Setup() override {};
    std::string getName() const override;
    std::string getHelp() const override;
    void execute(const Command_t& command) override;
};