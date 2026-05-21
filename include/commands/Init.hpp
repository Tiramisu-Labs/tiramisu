#pragma once

#include "ICommand.hpp"

class Init : public ICommand {
    private:

    public:
    Init();
    ~Init() override {};

    std::string getName() const override;
    std::string getHelp() const override;

    void execute(const Command_t& command) override;

};