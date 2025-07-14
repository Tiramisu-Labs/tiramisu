#include "../../include/commands/Host.hpp"

Host::Host() {}

std::string Host::getName() const { return "host"; }

std::string Host::getHelp() const {
    return "Usage: host <action> [arguments...]\n"
        "  Manages host configurations.\n"
        "  Actions: list, add <hostname> <ip>, remove <hostname>";
}

void Host::execute(const Command_t& command) {
    if (command.arguments.size() == 0) throw std::runtime_error("command missing\n" + getHelp());
    auto const sub_command = command.arguments.front();
    switch (host_cmds[sub_command])
    {
        case Commands::ADD: add(command.options); break;
        case Commands::LIST: list(); break;
        case Commands::INVALID: {
            std::cerr << "host -> " << sub_command << " unrecognized\n" + getHelp() << "\n";
            break;
        }
        default: break;
    }
}

static std::string readHosts() {
    std::ifstream read_file("hosts.yaml");
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
    if (readHosts().find(alias)) {
        std::cerr << alias << " already present\n";
        return ;
    }
    
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
    std::ofstream hosts_file("hosts.yaml", std::ios_base::app);
    if (!hosts_file.is_open()) {
        std::cerr << "An errror accured while opening file ~/.tiramisu/config/hosts.yaml\n";
    }
    hosts_file << alias << ":\n  " <<
        "host: " << host << "\n  " <<
        "user: " << user << "\n  " <<
        "password: " << password << "\n  " <<
        "port: " << port << "\n\n";
    hosts_file.close();
}
void Host::list() {
    std::ifstream hosts_file("hosts.yaml");
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
std::map<std::string, std::string> Host::getHostSpec(std::string& alias) {
    std::ifstream hosts_file("hosts.yaml");
    if (!hosts_file.is_open()) {
        std::cerr << "Warning: could not open hosts.yaml\n";
        exit(-1);
    }
    std::string line;
    std::string to_find = alias + ":";
    std::map<std::string, std::string> ret;
    while (std::getline(hosts_file, line)) {
        if (line == to_find) {
            while (std::getline(hosts_file, line)) {
                if (line.length() == 0) {
                    hosts_file.close();
                    return ret;
                }
                ret.insert({line.substr(0, line.find(':')), line.substr(line.find(": ") + 2)});
            }
        }
    }
    hosts_file.close();
    return ret;
}