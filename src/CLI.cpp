#include "../include/CLI.hpp"
#include "../include/SshHandler.hpp"
#include "../include/Parser.hpp"
#include "../include/Utils.hpp"
#include "../include/commands/Webserver.hpp"
#include "../include/commands/Host.hpp"
#include "../include/commands/Setup.hpp"
#include "../include/commands/Build.hpp"

#include <algorithm>
#include <ranges>
#include <fstream>
#include <map>
#include <filesystem>

static const std::map<std::string, Extensions> extensionsMap_ {
    {"c", Extensions::C},
    {"cpp", Extensions::CPP},
    {"rs", Extensions::RS},
    {"js", Extensions::JS},
    {"ts", Extensions::TS},
    {"go", Extensions::GO},
    {"py", Extensions::PY},
};

static std::map<std::string, Commands> commandsMap = {
    {"connect", Commands::CONNECT},
    {"host", Commands::HOST},
    {"build", Commands::BUILD},
    {"deploy", Commands::DEPLOY},
    {"setup", Commands::SETUP},
    {"help", Commands::HELP}
};

CLI::CLI(std::unique_ptr<Parser> parser, const std::string& env_path)
    : m_parser(std::move(parser)),
      m_sshHandler(std::make_unique<SshHandler>())
{
    if (!env_path.empty()) {
        loadEnvFile(env_path);
    }
    registerCommandFactories();
}

CLI::~CLI() {}

void CLI::loadEnvFile(const std::string& file_path) {
    std::ifstream file(file_path);
    
    if (!file.is_open()) {
        std::cerr << "Warning: could not open .env file: " << file_path << ". Using default/empty environment.\n";
        return ;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t eq_pos = line.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = line.substr(0, eq_pos);
            std::string value = line.substr(eq_pos + 1);
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            m_env[key] = value;
        }
    }
    std::cout << "Loaded " << m_env.size() << " environment variables from " << file_path << "\n";
}

void CLI::run() {
    try {
        Command_t parsed_command = m_parser->parse();
        processParsedCommand(parsed_command);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
        displayHelp();
    } catch (...) {
        std::cerr << "Unknown error\n";
        displayHelp();
    }
}

bool CLI::create_config_dir() {
    std::filesystem::path config_base_path;

#ifdef _WIN32
    const char* app_data_env = std::getenv("LOCALAPPDATA");
    if (app_data_env) {
        config_base_path = app_data_env;
        config_base_path /= "tiramisu";
    } else {
        std::cerr << "Warning: Variable LOCALAPPDATA not found on Windows." << std::endl;
        config_base_path = std::filesystem::current_path() / "mycli_config";
    }
#else
    const char* home_env = std::getenv("HOME");
    if (home_env) {
        config_base_path = home_env;
        config_base_path /= ".tiramisu";
    } else {
        std::cerr << "Warning: Variable HOME not found on Unix-like." << std::endl;
        config_base_path = std::filesystem::current_path() / ".mycli_config";
    }
#endif

    try {
        std::filesystem::create_directories(config_base_path);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "error while creating the config directory: " << e.what() << std::endl;
        return false;
    }
    return true;
}
    
void CLI::processParsedCommand(const Command_t& command) {
    std::unique_ptr<ICommand> currentCommandInstance = m_commandFactories[static_cast<size_t>(commandsMap[command.name])](m_sshHandler);
    try {
        currentCommandInstance->execute(command);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void CLI::executeConnect(const std::string& host, const std::string& user, const std::string& password) {
    std::cout << "Attempting to connect to SSH: " << user << "@" << host << " password: " << password << "...\n";
    std::cout << "SSH Connection established/simulated.\n";
}

void CLI::executeSSH(const std::string& command_to_execute) {
    if (!m_sshHandler->isConnected()) {
        throw std::runtime_error("Not connected to any SSH server. Use 'connect' first.");
    }
    std::cout << "Executing remote command: '" << command_to_execute << "'\n";
    std::cout << "Remote command executed/simulated.\n";
}

void CLI::executeSetEnv(const std::string& key, const std::string& value) {
    m_env[key] = value;
    std::cout << "Environment variable '" << key << "' set to '" << value << "'.\n";
}

void CLI::displayHelp(const std::string& command_name) {
    if (command_name.empty()) {
        std::cout << "CLI Usage:\n";
        std::cout << "  host <host> <user> <password> <alias> [--host <h>] [--user <u>] [--password <p>] [--alias <a>]\n";
        std::cout << "  setup <alias>\n";
        std::cout << "  ssh <command_string>\n";
        std::cout << "  set_env <key> <value>\n";
        std::cout << "  help [command]\n";
        std::cout << "\nFor more details, type 'help <command>'.\n";
    } else if (command_name == "connect") {
        std::cout << "Help for 'connect': Establishes an SSH connection.\n";
        std::cout << "  connect <host> <user> <password>\n";
        std::cout << "    Overrides can be provided with --host, --user, --password.\n";
    }
    else {
        std::cout << "No specific help available for command '" << command_name << "'.\n";
    }
}

void CLI::registerCommandFactories() {
    m_commandFactories.resize(static_cast<size_t>(Commands::SIZE));

    m_commandFactories[static_cast<size_t>(Commands::HOST)] =
        [&](std::unique_ptr<SshHandler>& sshHandler) { return std::make_unique<Host>(std::move(sshHandler)); };
    m_commandFactories[static_cast<size_t>(Commands::BUILD)] =
        [&](std::unique_ptr<SshHandler>& sshHandler) { (void)sshHandler; return std::make_unique<Build>(); };
    m_commandFactories[static_cast<size_t>(Commands::SETUP)] =
        [&](std::unique_ptr<SshHandler>& sshHandler) { return std::make_unique<Setup>(std::move(sshHandler)); };
    m_commandFactories[static_cast<size_t>(Commands::WEBSERVER)] =
        [&](std::unique_ptr<SshHandler>& sshHandler) { return std::make_unique<Webserver>(std::move(sshHandler)); };
}