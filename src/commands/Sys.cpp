#include <commands/Sys.hpp>
#include <iostream>
#include <stdexcept>
#include <SshHandler.hpp>
#include <Project.hpp>

Sys::Sys()
{
    commandsMap = {
        {"logs", [this](const Command& cmd) {
            std::string env = cmd.getOption("--env", "all");
            std::unordered_map<std::string, std::string> logOptions;
            logOptions["follow"] = cmd.hasFlag("-f") || cmd.hasFlag("--follow") ? "true" : "false";
            logOptions["lines"]  = cmd.getOption("-n", cmd.getOption("--lines", "50"));
            logOptions["target"] = cmd.getOption("--target", "caffeine");
            
            this->logs(env, logOptions);
        }},
        
        {"top", [this](const Command& cmd) {
            std::string env = cmd.getOption("--env", "all");
            this->top(env);
        }},
        
        {"df", [this](const Command& cmd) {
            std::string env = cmd.getOption("--env", "all");
            this->df(env);
        }},
        
        {"check", [this](const Command& cmd) {
            std::string env = cmd.getOption("--env", "all");
            bool useJson    = cmd.hasFlag("--json");
            this->check(env, useJson);
        }}
    };
}

Sys::~Sys() = default;

std::string Sys::getName() const { return "sys"; }
std::string_view Sys::getHelp() const { return SYS_HELP; }

void Sys::execute(const Command& command)
{
    if (command.help) {
        std::cout << getHelp();
        exit(0);
    }
    if (command.arguments.empty()) {
        throw std::runtime_error("error: missing command.\n" + std::string(getHelp()));
    }

    const auto subcommand = command.arguments[0];

    project = Project::getProject(command);

    auto it = commandsMap.find(subcommand);
    if (it != commandsMap.end()) {
        it->second(std::move(command));
    } else {
        throw std::runtime_error("error: unknown sys command variant '" + subcommand + "'.\n" + std::string(getHelp()));
    }
}

void Sys::logs(const std::string& env, const std::unordered_map<std::string, std::string>& options)
{
    auto buildJournalCommand = [&](bool forceDisableFollow = false) {
        std::string cmd = "journalctl";
        
        if (options.at("target") == "caffeine") {
            cmd += " -u caffeine.service";
        }
        
        cmd += " -n " + options.at("lines");
        
        // Block stream tracking if target is multiple nodes or explicitly disabled
        if (options.at("follow") == "true" && !forceDisableFollow) {
            cmd += " -f";
        } else {
            cmd += " --no-pager";
        }
        
        return cmd;
    };

    auto printTelemetryHeader = [&](const std::string& targetEnv, bool followActive) {
        std::cout << "[*] Streaming telemetry from environment: " << targetEnv << "\n";
        std::cout << "    Target Layer : " << options.at("target") << "\n";
        std::cout << "    Tail Lines   : " << options.at("lines") << "\n";
        std::cout << "    Follow Mode  : " << (followActive ? "true" : "false") << "\n\n";
    };

    if (!env.empty() && env != "all") {
        auto getEnv = project->getEnv(env);
        if (!getEnv) {
            throw std::runtime_error("error: provided env does not exist: " + env);
        }

        printTelemetryHeader(env, options.at("follow") == "true");

        try {
            SshHandler handler(getEnv->host, getEnv->user, getEnv->port, getEnv->key);
            std::string cmd = buildJournalCommand();
            handler.exec_remote_command(cmd);
        } catch (const std::exception& e) {
            throw std::runtime_error("error: connection failed for '" + env + "': " + e.what());
        }
    } else {
        std::cout << "[*] Aggregating multi-node telemetry matrix...\n";
        
        bool followRequested = (options.at("follow") == "true");
        if (followRequested) {
            std::cout << "[!] Warning: Cannot follow log streams sequentially across multiple nodes.\n";
            std::cout << "    Deactivating streaming mode for global aggregation sweep.\n\n";
        }

        for (const auto& [name, env] : project->getEnvs()) {
            std::cout << "=== Telemetry Snapshot: " << name << " ===" << std::endl;
            
            printTelemetryHeader(name, false);
            
            SshHandler handler(env.host, env.user, env.port, env.key);
            std::string cmd = buildJournalCommand(true);
            
            try {
                handler.exec_remote_command(cmd);
            } catch (const std::exception& e) {
                std::cerr << "[!] Failed to query node '" << name << "': " << e.what() << "\n";
            }
            
            std::cout << "===========================================\n\n";
        }
    }
}

void Sys::top(const std::string& env)
{
    auto buildTopCommand = []() {
        return std::string(
            "echo '--- System Performance Snapshot ---' && "
            "top -bn1 | head -n 5 && "
            "echo '' && "
            "echo '--- Hardware Metrics ---' && "
            "if [ -f /sys/class/thermal/thermal_zone0/temp ]; then "
            "  echo \"SoC Temperature: $(awk '{print $1/1000}' /sys/class/thermal/thermal_zone0/temp)°C\"; "
            "else "
            "  echo 'SoC Temperature: N/A (Non-thermal zone device)'; "
            "fi && "
            "echo '' && "
            "echo '--- Caffeine Worker Memory Maps (.so) ---' && "
            "if pgrep caffeine > /dev/null; then "
            "  for pid in $(pgrep caffeine); do "
            "    echo \"Worker Process [PID: $pid]\"; "
            "    cat /proc/$pid/maps 2>/dev/null | grep '\\.so' | awk '{print \"  [\" $1 \"] -> \" $6}' | sort -u; "
            "  done; "
            "else "
            "  echo '[!] No active Caffeine worker processes detected.'; "
            "fi"
        );
    };

    auto printTopHeader = [](const std::string& targetEnv) {
        std::cout << "[*] Launching terminal diagnostic dashboard for environment: " << targetEnv << "\n\n";
    };

    if (!env.empty() && env != "all") {
        auto getEnv = project->getEnv(env);
        if (!getEnv) {
            throw std::runtime_error("error: provided env does not exist: " + env);
        }

        printTopHeader(env);

        SshHandler handler(getEnv->host, getEnv->user, getEnv->port, getEnv->key);
        std::string cmd = buildTopCommand();
        
        handler.exec_remote_command(cmd);
    } 
    else {
        std::cout << "[*] Aggregating multi-node diagnostic matrix...\n\n";

        for (const auto& [name, env] : project->getEnvs()) {
            std::cout << "======================================================================\n";
            std::cout << " Node Cluster Diagnostic Matrix: " << name << "\n";
            std::cout << "======================================================================\n";
            
            printTopHeader(name);
            
            try {
                SshHandler handler(env.host, env.user, env.port, env.key);
                handler.exec_remote_command(buildTopCommand());
            } catch (const std::exception& e) {
                std::cerr << "[!] Failed to pull diagnostic data from node '" << name << "': " << e.what() << "\n";
            }
            
            std::cout << "======================================================================\n\n";
        }
    }
}

// TO-CHECK! Written by Gemini the stupid
void Sys::df(const std::string& env)
{
    std::string dfCmd = "echo '--- File System Disk Space Utilization ---' && df -h -x tmpfs -x devtmpfs";

    if (!env.empty() && env != "all") {
        auto getEnv = project->getEnv(env);
        if (!getEnv) {
            throw std::runtime_error("error: provided env does not exist: " + env);
        }

        std::cout << "[*] Querying remote storage layout for environment: " << env << "\n\n";
        try {
            SshHandler handler(getEnv->host, getEnv->user, getEnv->port, getEnv->key);
            handler.exec_remote_command(dfCmd);
        } catch (const std::exception& e) {
            throw std::runtime_error("error: connection failed for '" + env + "': " + e.what());
        }
    } 
    else {
        std::cout << "[*] Aggregating multi-node storage matrix...\n\n";
        for (const auto& [name, targetEnv] : project->getEnvs()) {
            std::cout << "=== Storage Allocation Capacity: " << name << " ===\n";
            try {
                SshHandler handler(targetEnv.host, targetEnv.user, targetEnv.port, targetEnv.key);
                handler.exec_remote_command(dfCmd);
            } catch (const std::exception& e) {
                std::cerr << "[!] Failed to pull storage data from node '" << name << "': " << e.what() << "\n";
            }
            std::cout << "===================================================\n\n";
        }
    }
}

void Sys::check(const std::string& env, bool json)
{
    std::string cmdJson = R"(echo "{\"engine\": \"$(systemctl is-active caffeine.service 2>/dev/null || echo inactive)\", \"workers\": $(pgrep -c caffeine || echo 0), \"load_avg\": \"$(cat /proc/loadavg 2>/dev/null | awk '{print $1 " " $2 " " $3}' || echo '0.00 0.00 0.00')\"}")";

    std::string cmdText = R"(
        echo "--- Caffeine Engine Sanity Matrix ---" && \
        echo "Engine Status  : $(systemctl is-active caffeine.service 2>/dev/null || echo 'INACTIVE / NOT FOUND')\" && \
        echo "Worker Count   : $(pgrep -c caffeine || echo 0) active processes" && \
        echo "Kernel Load Avg: $(cat /proc/loadavg 2>/dev/null | awk '{print $1 " " $2 " " $3}' || echo 'N/A')\"
    )";

    std::string targetCmd = json ? cmdJson : cmdText;

    if (!env.empty() && env != "all") {
        auto getEnv = project->getEnv(env);
        if (!getEnv) {
            if (json) {
                std::cout << "{\"error\": \"provided env does not exist: " << env << "\"}\n";
                return;
            }
            throw std::runtime_error("error: provided env does not exist: " + env);
        }

        if (!json) {
            std::cout << "[*] Running rapid engine sanity audit on environment: " << env << "\n\n";
        }

        try {
            SshHandler handler(getEnv->host, getEnv->user, getEnv->port, getEnv->key);
            handler.exec_remote_command(targetCmd);
        } catch (const std::exception& e) {
            if (json) {
                std::cout << "{\"engine\": \"unreachable\", \"error\": \"" << e.what() << "\"}\n";
            } else {
                throw std::runtime_error("error: connection failed for '" + env + "': " + e.what());
            }
        }
    } 
    else {
        auto envs = project->getEnvs();

        if (json) {
            std::cout << "{\n  \"cluster_health\": {\n";
            size_t idx = 0;
            
            for (const auto& [name, env] : envs) {
                std::cout << "    \"" << name << "\": ";
                std::cout.flush();

                try {
                    SshHandler handler(env.host, env.user, env.port, env.key);
                    handler.exec_remote_command(targetCmd);
                } catch (const std::exception& e) {
                    std::cout << "{\"engine\": \"unreachable\", \"error\": \"" << e.what() << "\"}";
                }

                if (++idx < envs.size()) {
                    std::cout << ",\n";
                } else {
                    std::cout << "\n";
                }
            }
            std::cout << "  }\n}\n";
        } else {
            std::cout << "[*] Running global rapid engine sanity audit across cluster matrix...\n\n";

            for (const auto& [name, env] : envs) {
                std::cout << "===========================================\n";
                std::cout << " Sanity Health Audit Node: " << name << "\n";
                std::cout << "===========================================\n";

                SshHandler handler(env.host, env.user, env.port, env.key);
                try {
                    handler.exec_remote_command(targetCmd);
                } catch (const std::exception& e) {
                    std::cerr << "[!] Node Audit Failed: " << e.what() << "\n";
                }
                std::cout << "===========================================\n\n";
            }
        }
    }
}