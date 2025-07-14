#pragma once

#include "ICommand.hpp"
#include <memory>

class SshHandler;

class Setup : public ICommand {
    private:
    std::unique_ptr<SshHandler> m_sshHandler;
    public:
    Setup();
    Setup(std::unique_ptr<SshHandler>&& handler);
    ~Setup() override {};
    std::string getName() const override;
    std::string getHelp() const override;
    void execute(const Command_t& command) override;
    
};