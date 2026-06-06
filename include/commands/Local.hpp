#pragma once

#include "ICommand.hpp"
#include <map>
#include <functional>

inline constexpr std::string_view LOCAL_HELP = R"(
Tiramisu Local Virtualized Cloud Orchestration

Usage:
  tiramisu local <command> [options]

Commands:
  start     Initialize and spin up the containerized local edge environments.
            Automatically provisions 'env-alpha' (HTTP: 8081, SSH: 2221) and 
            'env-beta' (HTTP: 8082, SSH: 2222) with individual Postgres backends.
            Self-heals and unpacks internal templates to ~/.tiramisu/ if missing.

  stop      Safely halt all running local Tiramisu environment nodes and their
            associated databases without erasing your functional state or data.

  status    Render an administrative matrix of your running local cluster.
            Verifies health, container uptimes, and active localhost port maps.

  clean     The ultimate reset switch. Completely tears down the local cluster,
            wipes out all deployed test `.so` binaries, and drops all database
            volumes back to a pristine, blank canvas.

Options:
  --build     Force Docker to completely rebuild the runtime images from scratch.
              (Applicable to: start)
              
  -y, --yes   Suppresses terminal warnings and bypasses security verification locks.
              (Applicable to: clean)

Examples:
  # Spin up or self-heal the local cloud simulation
  $ tiramisu local start

  # Force-rebuild your internal Nginx or base runtime images
  $ tiramisu local start --build

  # Clear out all test states and database data instantly
  $ tiramisu local clean -y
)";

class Local : public ICommand
{
    public:
    Local();
    ~Local();

    std::string getName() const override;
    std::string_view getHelp() const override;
    void execute(const Command& command) override;

    void start(const Command&& command);
    void stop(const Command&& command);
    void status(const Command&& command);
    void clean(const Command&& command);

    private:
    std::map<std::string, std::function<void(const Command&& command)>> commandsMap = {
        {"start", [this](const Command&& cmd) {start(std::move(cmd)); }},
        {"stop", [this](const Command&& cmd) {stop(std::move(cmd)); }},
        {"status", [this](const Command&& cmd) {status(std::move(cmd)); }},
        {"clean", [this](const Command&& cmd) {clean(std::move(cmd)); }},
    };
};