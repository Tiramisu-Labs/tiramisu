#include <commands/Caffeine.hpp>
#include <SshHandler.hpp>
#include <Project.hpp>
#include <iostream>
#include <algorithm>

std::string Caffeine::getName() const { return "caffeine"; }

std::string_view Caffeine::getHelp() const {
    return CAFFEINE_HELP;
}

void Caffeine::execute(const Command& command) {
    if (command.help) {
        std::cout << getHelp() << std::endl; 
        return;
    }
    if (command.arguments.empty()) {
        throw std::runtime_error(std::string(getHelp()));
    }
    
    auto const sub_command = command.arguments.front();
    
    if (!commandsMap.contains(sub_command)) {
        std::cerr << "error: unknown caffeine subcommand '" << sub_command << "'\n\n" << getHelp();
        return;
    }

    commandsMap[sub_command](std::move(command));
}

void Caffeine::config(const Command&& command) {
    if (command.arguments.size() < 2) {
        std::cerr << "error: an env name is required.\n\n" << getHelp();
        return;
    }

    const auto env_name = command.arguments[1];
    auto project = Project::getProject(command);
    auto getEnv = project->getEnv(env_name);

    if (!getEnv) {
        std::cerr << "error: environment name '" << env_name << "' not found in tiramisu.yaml\n";
        return;
    }

    SshHandler handler(getEnv->host, getEnv->user, getEnv->port, getEnv->key);

    if (command.options.contains("--get") || std::find(command.arguments.begin(), command.arguments.end(), "--get") != command.arguments.end()) {
        std::cout << "Fetching Caffeine runtime configuration from environment '" << env_name << "'...\n";
        handler.exec_remote_command("cat /etc/caffeine/caffeine.conf 2>/dev/null || echo 'Configuration file not found.'");
        return;
    }

    bool mutated = false;
    std::string remote_cmd = "bash -c '";

    if (command.options.contains("--workers")) {
        std::string workers = command.options.find("--workers")->second;
        remote_cmd += "sed -i \"s/^workers=.*/workers=" + workers + "/\" /etc/caffeine/caffeine.conf || echo \"workers=" + workers + "\" >> /etc/caffeine/caffeine.conf; ";
        mutated = true;
    }

    if (command.options.contains("--port")) {
        std::string port = command.options.find("--port")->second;
        remote_cmd += "sed -i \"s/^port=.*/port=" + port + "/\" /etc/caffeine/caffeine.conf || echo \"port=" + port + "\" >> /etc/caffeine/caffeine.conf; ";
        mutated = true;
    }

    if (command.options.contains("--tls-cert")) {
        std::string cert = command.options.find("--tls-cert")->second;
        remote_cmd += "sed -i \"s|^tls_cert=.*|tls_cert=" + cert + "|\" /etc/caffeine/caffeine.conf || echo \"tls_cert=" + cert + "\" >> /etc/caffeine/caffeine.conf; ";
        mutated = true;
    }

    if (command.options.contains("--tls-key")) {
        std::string key_path = command.options.find("--tls-key")->second;
        remote_cmd += "sed -i \"s|^tls_key=.*|tls_key=" + key_path + "|\" /etc/caffeine/caffeine.conf || echo \"tls_key=" + key_path + "\" >> /etc/caffeine/caffeine.conf; ";
        mutated = true;
    }

    remote_cmd += "'";

    if (!mutated) {
        std::cerr << "error: no modification flags or --get option provided.\n\n" << getHelp();
        return;
    }

    std::cout << "Updating Caffeine configuration on environment '" << env_name << "'...\n";
    handler.exec_remote_command(remote_cmd);

    std::cout << "Sending SIGHUP to trigger a zero-downtime hot-reload... ";
    std::cout.flush();
    handler.exec_remote_command("pkill -HUP caffeine || systemctl reload caffeine");
    std::cout << "Done!\n";
}

void Caffeine::start(const Command&& command) {
    if (command.arguments.size() < 2) {
        std::cerr << "error: an env name is required.\n\n" << getHelp();
        return;
    }

    const auto env_name = command.arguments[1];
    auto project = Project::getProject(command);
    auto getEnv = project->getEnv(env_name);

    if (!getEnv) {
        std::cerr << "error: environment name '" << env_name << "' not found in tiramisu.yaml\n";
        return;
    }

    SshHandler handler(getEnv->host, getEnv->user, getEnv->port, getEnv->key);
    std::cout << "Starting Caffeine edge runtime service on environment '" << env_name << "'...\n";

    std::string check_systemd = "ps -p 1 -o comm= 2>/dev/null";
    std::string start_cmd = "if [ \"$(ps -p 1 -o comm=)\" = \"systemd\" ]; then "
                            "  systemctl start caffeine; "
                            "else "
                            "  nohup /usr/local/bin/caffeine --config /etc/caffeine/caffeine.conf > /var/log/caffeine.log 2>&1 & "
                            "fi";

    handler.exec_remote_command(start_cmd);
}

void Caffeine::status(const Command&& command) {
    if (command.arguments.size() < 2) {
        std::cerr << "error: an env name is required.\n\n" << getHelp();
        return;
    }

    const auto env_name = command.arguments[1];
    auto project = Project::getProject(command);
    auto getEnv = project->getEnv(env_name);

    SshHandler handler(getEnv->host, getEnv->user, getEnv->port, getEnv->key);
    std::cout << "Querying Caffeine operational state on environment '" << env_name << "':\n\n";

    std::string status_cmd = "if [ \"$(ps -p 1 -o comm=)\" = \"systemd\" ]; then "
                             "  systemctl status caffeine; "
                             "else "
                             "  pgrep -fl caffeine && echo -e '\\nStatus: RUNNING (Standalone Process)' || echo -e 'Status: STOPPED'; "
                             "fi";

    handler.exec_remote_command(status_cmd);
}

void Caffeine::stop(const Command&& command) {
    if (command.arguments.size() < 2) {
        std::cerr << "error: an env name is required.\n\n" << getHelp();
        return;
    }

    const auto env_name = command.arguments[1];
    auto project = Project::getProject(command);
    auto getEnv = project->getEnv(env_name);

    if (!getEnv) {
        std::cerr << "error: environment name '" << env_name << "' not found in tiramisu.yaml\n";
        return;
    }

    SshHandler handler(getEnv->host, getEnv->user, getEnv->port, getEnv->key);
    std::cout << "Stopping Caffeine edge runtime service on environment '" << env_name << "'...\n";
    handler.exec_remote_command("systemctl stop caffeine");
}

void Caffeine::restart(const Command&& command) {
    if (command.arguments.size() < 2) {
        std::cerr << "error: an env name is required.\n\n" << getHelp();
        return;
    }

    const auto env_name = command.arguments[1];
    auto project = Project::getProject(command);
    auto getEnv = project->getEnv(env_name);

    if (!getEnv) {
        std::cerr << "error: environment name '" << env_name << "' not found in tiramisu.yaml\n";
        return;
    }

    SshHandler handler(getEnv->host, getEnv->user, getEnv->port, getEnv->key);
    std::cout << "Hard cycling Caffeine system sockets and workers on environment '" << env_name << "'...\n";
    handler.exec_remote_command("systemctl restart caffeine");
}