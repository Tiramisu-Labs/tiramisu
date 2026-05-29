#pragma once

#include "ICommand.hpp"
#include <memory>

class SshHandler;

inline constexpr std::string_view SETUP_HELP = R"(
   "Usage: setup <host> [arguments...]\n"
        "  install packages inside the remote host.\n"
        "  Default: nginx";
)";

class Setup : public ICommand {
    public:
    Setup() = default;
    ~Setup() override {};
    std::string getName() const override;
    std::string_view getHelp() const override;
    void execute(const Command& command) override;
    
};
