#pragma once

#include "ICommand.hpp"

inline constexpr std::string_view BUILD_SERVE = R"(
Usage: serve <folder> [options...]
    Serve a folder containing a static site on the remote host.
    The site will be available using nginx as web server.
)";

class Serve: public ICommand
{
    public:
        Serve() = default;
        ~Serve() override = default;

        std::string getName() const override;
        std::string_view getHelp() const override;
        void execute(const Command& command) override;
};
