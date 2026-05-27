#pragma once

#include "ICommand.hpp"

inline constexpr std::string_view INSTALL_HELP = R"(
    "Try to install the desired package:\n"
        "  emscripten   install emscripten to compile C and C++ into wasm";
)";

class Install : public ICommand {
    public:
    Install();
    ~Install() override {};
    std::string getName() const override;
    std::string_view getHelp() const override;
    void execute(const Command& command) override;
};
