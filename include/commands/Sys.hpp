#pragma once

#include "ICommand.hpp"

#include <string_view>
#include <functional>
#include <Utils.hpp>

inline constexpr std::string_view SYS_HELP = R"(
Tiramisu System Observability & Diagnostics

Usage:
  tiramisu sys <command> [options]

Commands:
  logs      Stream unified system and engine telemetry back to your terminal.
            Provides native visualization of active runtime execution lines,
            request access paths, and internal process exceptions.

  top       Render a live, interactive terminal diagnostic dashboard.
            Tracks real-time CPU core loads, physical SoC temperatures, and 
            the exact virtual memory addresses where active handler `.so` 
            binaries are dynamically mapped into the worker process spaces.

  df        Query file-system metrics across the remote target hardware.
            Inspects byte capacities and sectors on the primary Micro-SD boot 
            drive as well as dedicated external high-performance SSD storage blocks.

  check     Execute a non-interactive, rapid sanity audit on the edge node.
            Instantly verifies worker pool health, active listener status, 
            allocated memory boundaries, and overall kernel socket load balancing.

Options:
  -f, --follow       Persistently track real-time runtime log updates (tail -f).
                     (Applicable to: logs)

  -n, --lines <int>  Specify the depth window for historical trailing log lines.
                     (Default: 50) (Applicable to: logs)

  --target <layer>   Filter logs down to a specific subsystem: `caffeine` or `system`.
                     (Default: caffeine) (Applicable to: logs)

  --json             Format the operational health matrix into a structured JSON 
                     payload for automated monitoring and scraping tools.
                     (Applicable to: check)
  
  --env <env_name>   Inspect only the target env. Default to all

Examples:
  # Check live diagnostic performance of your edge hardware
  $ tiramisu sys top

  # Tail streaming execution outputs inside Caffeine's memory layer
  $ tiramisu sys logs -f --target caffeine

  # Fetch an automated health check payload to audit node integrity
  $ tiramisu sys check --json
)";

class Sys : public ICommand
{
    private:
    // commands
    void logs(const std::string& env, const std::unordered_map<std::string, std::string>& options);
    void top(const std::string& env);
    void df(const std::string& env);
    void check(const std::string& env, bool json);
    std::unique_ptr<class Project> project;
    public:
    Sys();
    ~Sys() override;
    std::string getName() const override;
    std::string_view getHelp() const override;
    void execute(const Command& command) override;

    std::unordered_map<std::string, std::function<void(const Command&& command)>> commandsMap;
};