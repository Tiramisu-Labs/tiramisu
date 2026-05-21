#pragma once

#include "ICommand.hpp"

class Create : public ICommand
{
private:
public:
    Create();
    ~Create() override {};

    std::string getName() const override;
    std::string getHelp() const override;

    void execute(const Command_t& command) override;
};
