#include "../../include/commands/Host.hpp"
#include "../../include/SshHandler.hpp"

Host::Host() {}
Host::Host(std::unique_ptr<SshHandler>&& handler) : m_sshHandler(std::move(handler)) {}

std::string Host::getName() const { return "host"; }

std::string Host::getHelp() const {
    return "Usage: host <action> [arguments...]\n"
        "  Manages host configurations.\n"
        "  host add [--host=<IP/DNS>] [--user=<user>] [--password=<password>] [--port=<port>] [--alias=<alias>]: add a new host to the hosts list"
        "  host list: list stored hosts"
        "  host test [--alias=<alias>]: test connection with the specified host";
}

void Host::execute(const Command_t& command) {
    if (command.arguments.size() == 0) throw std::runtime_error(getHelp());
    auto const sub_command = command.arguments.front();
    switch (host_cmds[sub_command])
    {
        case Commands::HELP: getHelp(); break;
        case Commands::ADD: add(command.options); break;
        case Commands::LIST: list(); break;
        case Commands::TEST: test(command.options); break;
        case Commands::INVALID: {
            std::cerr << "host -> " << sub_command << " unrecognized\n" + getHelp() << "\n";
            break;
        }
        default: break;
    }
}

static std::string readHosts() {
    std::ifstream read_file(std::string(std::getenv("HOME")) + "/.config/tiramisu/hosts.yaml");
    if (!read_file.is_open()) {
        std::cerr << "Warning: could not open hosts.yaml\n";
        exit(1) ;
    }
    std::string line;
    std::string ret = "";
    while (std::getline(read_file, line)) {
        ret += line + "\n";
    }
    read_file.close();
    return ret;
}

void Host::add(std::unordered_map<std::string, std::string> options) {
    std::string host = "";
    std::string user = "";
    std::string password = "";
    std::string port = "22";
    std::string alias = host;
    
    std::cout << "parse command\n";
    for (const auto& opt : options) {
        if (opt.first == "--host") host = opt.second;
        else if (opt.first == "--user") user = opt.second;
        else if (opt.first == "--password") password = opt.second;
        else if (opt.first == "--port") port = opt.second;
        else if (opt.first == "--alias") alias = opt.second;
        else {
            std::cerr << "Warning: Unrecognized option '" << opt.first << "' for connect command.\n";
        }
    }

    if (readHosts().find(alias) != std::string::npos) {
        std::cerr << alias << " already present\n";
        return ;
    }
    
    m_sshHandler->fillSshHandler(host, password, user, port);
    try {
        test();
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << "\n";
        exit(1);
    }

    std::string arch = getArch();
    std::string hosts_path = std::string(std::getenv("HOME")) + "/.config/tiramisu";
    if (!std::filesystem::exists(hosts_path)) {
        std::cout << "Configuration directory does not exists: " << hosts_path << std::endl;
        if (std::filesystem::create_directories(hosts_path)) {
            std::cout << "Directory succesfully created: " << hosts_path << std::endl;
        } else {
            std::cerr << "Error: couldn't create directory: " << hosts_path << std::endl;
            return ;
        }
    }
    std::ofstream hosts_file(std::string(std::getenv("HOME")) + "/.config/tiramisu/hosts.yaml", std::ios_base::app);
    if (!hosts_file.is_open()) {
        std::cerr << "An errror accured while opening file ~/.tiramisu/config/hosts.yaml\n";
    }
    hosts_file << alias << ":\n  " <<
        "host: " << host << "\n  " <<
        "user: " << user << "\n  " <<
        "password: " << password << "\n  " <<
        "port: " << port << "\n  " <<
        "arch: " << arch << "\n\n";
    hosts_file.close();
}

void Host::list() {
    std::ifstream hosts_file(std::string(std::getenv("HOME")) + "/.config/tiramisu/hosts.yaml");
    if (!hosts_file.is_open()) {
        std::cerr << "Warning: could not open hosts.yaml\n";
        return ;
    }
    std::string line;
    while (std::getline(hosts_file, line)) {
        std::cout << line << "\n";
    }
    hosts_file.close();
}

std::string Host::getArch() const { return m_sshHandler->getArch(); }

void Host::test()
{   
    if (m_sshHandler->sshConnect()) {
        m_sshHandler->sshDisconnect();
    } else {
        throw std::runtime_error("remote host specs wrongs!");
    }
}

void Host::test(const std::unordered_map<std::string, std::string> &options)
{
    const auto alias = options.find("--alias");
    if (alias == options.end()) throw std::runtime_error("alias not found");
    try {
        m_sshHandler->fillSshHandler(alias->second);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    m_sshHandler->exec_remote_command({"arch"});
}