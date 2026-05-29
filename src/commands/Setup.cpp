#include <commands/Setup.hpp>
#include <SshHandler.hpp>

#include <iostream>
#include <algorithm>

std::string Setup::getName() const { return "setup"; }

std::string_view Setup::getHelp() const {
    return SETUP_HELP;
}

void Setup::execute(const Command & command) {
    // DEBUG prints
    std::cout << command.name << std::endl;
    const auto& options = command.options;

    for (auto [x, y] : options) {
        std::cout << "key: " << x << " value: " << y << std::endl;
    }

    std::cout << "Preparing the remote host\n";

    const auto handler = std::make_unique<SshHandler>();
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
        handler->fillSshHandler(alias_it->second);
        handler->upload("install_nginx.sh");
        handler->exec_remote_command("chmod 777 install_nginx.sh && bash install_nginx.sh " + alias_it->second);
        handler->exec_remote_command("rm install_nginx.sh");
        handler->exec_remote_command("rm install_nginx.sh");
        handler->exec_remote_command("rm nginx-1.28.0");
        handler->exec_remote_command("rm nginx-1.28.0.tar.gz");
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}
