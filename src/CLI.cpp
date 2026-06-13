#include <CLI.hpp>
#include <SshHandler.hpp>
#include <Parser.hpp>
#include <Utils.hpp>
#include <commands/Host.hpp>
#include <commands/Init.hpp>
#include <commands/Build.hpp>
#include <commands/Create.hpp>
#include <commands/Local.hpp>
#include <commands/Sys.hpp>
#include <algorithm>
#include <ranges>
#include <fstream>
#include <map>
#include <filesystem>

static const std::map<std::string, Extensions> extensionsMap {
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
    {"init", Commands::INIT},
    {"host", Commands::HOST},
    {"create", Commands::CREATE},
    {"build", Commands::BUILD},
    {"deploy", Commands::DEPLOY},
    {"help", Commands::HELP},
    {"local", Commands::LOCAL},
    {"sys", Commands::SYS}
};

static std::map<std::string, std::string_view> commandsHelp = {
    {"init", INIT_HELP},
    {"host", HOST_HELP},
    {"create", CREATE_HELP},
    {"build", BUILD_HELP},
    {"local", LOCAL_HELP},
    {"sys", SYS_HELP}
};

CLI::CLI(std::unique_ptr<Parser> parser, const std::string& env_path)
    : m_parser(std::move(parser))
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
        Command parsed_command = m_parser->parse();
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
    
void CLI::processParsedCommand(const Command& command) {
    std::unique_ptr<ICommand> currentCommandInstance = m_commandFactories[static_cast<size_t>(commandsMap[command.name])]();
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

void CLI::executeSSH(const std::string& Commando_execute) {
    if (!m_sshHandler->isConnected()) {
        throw std::runtime_error("Not connected to any SSH server. Use 'connect' first.");
    }
    std::cout << "Executing remote command: '" << Commando_execute << "'\n";
    std::cout << "Remote command executed/simulated.\n";
}

void CLI::executeSetEnv(const std::string& key, const std::string& value) {
    m_env[key] = value;
    std::cout << "Environment variable '" << key << "' set to '" << value << "'.\n";
}

void CLI::displayHelp(const std::string& command_name) {
    if (command_name.empty()) {
        std::cout << "Tiramisu Edge Engine CLI Tool\n\n";
        std::cout << "usage:\n";
        std::cout << "  tiramisu <command> [arguments] [options]\n\n";
        
        std::cout << "Core Node Lifecycle Commands:\n";
        std::cout << "  host       Register or provision a remote edge target node hardware configuration.\n";
        std::cout << "  init       Initialize a new local Tiramisu project workspace framework.\n";
        std::cout << "  build      Compile native handler code modules into deployable binary files.\n";
        std::cout << "  create     Scaffold code template configurations for a new custom dynamic handler.\n";
        std::cout << "  setup      Bootstrap runtime dependencies, systemd profiles, and engines on a node.\n\n";

        std::cout << "Execution & Environment Management:\n";
        std::cout << "  local      Execute runtime handling operations locally on your machine for debugging.\n";
        std::cout << "  ssh        Execute a raw bash command string securely on a remote workspace node.\n";
        std::cout << "  connect    Manually bridge a persistent, interactive SSH terminal session to a host.\n";
        std::cout << "  set_env    Bind explicit environment profile keys to custom operational runtime values.\n\n";

        std::cout << "Observability & System Monitoring:\n";
        std::cout << "  sys        Stream core engine telemetries, monitor CPU loads, and audit overall health.\n\n";

        std::cout << "Global Utilities:\n";
        std::cout << "  help       Display this operational command summary or explicit command deep-dives.\n\n";
        
        std::cout << "For more detailed insights regarding a mechanism, type 'tiramisu help <command>'.\n";
        return;
    }

    auto it = commandsHelp.find(command_name);
    if (it != commandsHelp.end()) {
        std::cout << it->second << std::endl;
    } else {
        std::cerr << "[!] Error: Unknown command identifier '" << command_name << "'.\n";
        std::cout << "Type 'tiramisu help' to review valid system execution options.\n";
    }
}

void CLI::registerCommandFactories() {
    m_commandFactories.resize(static_cast<size_t>(Commands::SIZE));

    m_commandFactories[static_cast<size_t>(Commands::HOST)] = [&]() { return std::make_unique<Host>(); };
    m_commandFactories[static_cast<size_t>(Commands::INIT)] = [&]() { return std::make_unique<Init>(); };
    m_commandFactories[static_cast<size_t>(Commands::BUILD)] = [&]() { return std::make_unique<Build>(); };
    m_commandFactories[static_cast<size_t>(Commands::CREATE)] = [&]() { return std::make_unique<Create>(); };
    m_commandFactories[static_cast<size_t>(Commands::LOCAL)] = [&]() { return std::make_unique<Local>(); };
    m_commandFactories[static_cast<size_t>(Commands::SYS)] = [&]() { return std::make_unique<Sys>(); };
    // deploy...
}