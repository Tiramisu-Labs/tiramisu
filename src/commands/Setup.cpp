#include "../../include/commands/Setup.hpp"
#include "../../include/SshHandler.hpp" 

#include <iostream>
#include <algorithm>

Setup::Setup() {}
Setup::Setup(std::unique_ptr<SshHandler>&& handler) : m_sshHandler(std::move(handler)) {}

std::string Setup::getName() const { return "setup"; }

std::string Setup::getHelp() const {
    return "Usage: setup <host> [arguments...]\n"
        "  install packages inside the remote host.\n"
        "  Default: nginx";
}

void Setup::execute(const Command_t & command) {
    // DEBUG prints
    std::cout << command.name << std::endl;
    const auto& options = command.options;

    for (auto [x, y] : options) {
        std::cout << "key: " << x << " value: " << y << std::endl;
    }

    std::cout << "Preparing the remote host\n";
    try {
        const auto alias_it = options.find("--alias");
        if (alias_it == options.end()) throw std::runtime_error("alias option is missing!");
        std::string web_server = "nginx";
        std::string version = "1.28.0";
        const auto server_it = options.find("--web-server");
        if (server_it != options.end()) web_server = server_it->second;
        const auto version_it = options.find("--version");
        if (version_it != options.end()) version = version_it->second;
        std::cout << "installing " << web_server << " on remote server. Version: " << version << "\n";
        m_sshHandler->fillSshHandler(alias_it->second);
        m_sshHandler->upload("scripts/install_nginx.sh");
        m_sshHandler->exec_remote_commands({
            "chmod +x install_nginx.sh && bash install_nginx.sh",
            "rm install_nginx.sh"
        });
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}