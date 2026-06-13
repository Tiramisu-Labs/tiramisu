#pragma once

#include "ICommand.hpp"
#include <functional>

inline constexpr std::string_view CAFFEINE_HELP = R"(
Usage: tiramisu caffeine <action> <env_name> [options]

Manage and configure the native, high-performance Caffeine edge runtime engine
using target profile definitions registered inside your local tiramisu.yaml.

Actions:
  config               Inspect, query, or mutate active runtime engine parameters.
  start                Launch the Caffeine edge server daemon on the remote environment.
  stop                 Gracefully halt the running Caffeine edge server daemon.
  restart              Hard cycle the engine daemon (re-allocates pools & re-binds sockets).
  status               Query the operational and administrative state of the service.

Options for 'config':
  --get                Fetch and display the parsed live configuration from the environment.
  --workers <int>      Configure the size of the isolated worker process pool.
  --port <int>         Override the public network facing HTTP binding port.
  --tls-cert <path>    Specify the path to the public SSL certificate for native HTTPS.
  --tls-key <path>     Specify the path to the private SSL key file for native HTTPS.

Global Context Flags:
  --verbose            Outputs raw shell executions and SFTP transfer logs to the terminal.

Description:
  The 'caffeine' namespace maps directly to an environment profile in tiramisu.yaml. 
  The CLI implicitly handles connection handshakes (resolving host, user, port, 
  and keys under the hood) based on the provided <env_name>.

  Modifying properties via 'caffeine config' automatically issues a lightweight 
  runtime signal (SIGHUP) to the master process on the target architecture to trigger 
  hot-swapping and worker adjustments on-the-fly with ZERO connection downtime.

Examples:
  tiramisu caffeine status test2
  tiramisu caffeine config test2 --get
  tiramisu caffeine config test2 --workers 4 --port 443 --verbose
)";

class Caffeine : public ICommand {
private:
    void config(const Command&& command);
    void start(const Command&& command);
    void stop(const Command&& command);
    void restart(const Command&& command);
    void status(const Command&& command);

public:
    Caffeine() = default;
    ~Caffeine() override = default;

    std::string getName() const override;
    std::string_view getHelp() const override;
    void execute(const Command& command) override;

    std::map<std::string, std::function<void(const Command&& command)>> commandsMap = {
        {"config",  [this](const Command&& cmd) { config(std::move(cmd)); }},
        {"start",   [this](const Command&& cmd) { start(std::move(cmd)); }},
        {"stop",    [this](const Command&& cmd) { stop(std::move(cmd)); }},
        {"restart", [this](const Command&& cmd) { restart(std::move(cmd)); }},
        {"status",  [this](const Command&& cmd) { status(std::move(cmd)); }}
    };
};