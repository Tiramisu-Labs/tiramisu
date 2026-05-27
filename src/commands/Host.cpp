#include <commands/Host.hpp>
#include <SshHandler.hpp>

Host::Host() : m_sshHandler(std::make_unique<SshHandler>()) {}

std::string Host::getName() const { return "host"; }

std::string_view Host::getHelp() const {
    return HOST_HELP;
}

void Host::execute(const Command& command) {
    if (command.help) {
        std::cout << getHelp() << std::endl; 
        return ;
    }
    if (command.arguments.size() == 0) throw std::runtime_error(std::string(getHelp()));
    auto const sub_command = command.arguments.front();
}

void Host::add(const Command&& command) {
    if (!command.options.contains("--ip")) {
        std::cerr << "tiramisu: error: an ip is required.\n" << getHelp() << std::endl;
        return;
    }
    const std::string ip = command.options.find("--ip")->second;
    std::string user = command.options.contains("--user") ? command.options.find("--user")->second : "root";
    std::string key = command.options.contains("--key") ? command.options.find("--key")->second : "~/.ssh/id_rsa";
    std::string port = command.options.contains("--port") ? command.options.find("--port")->second : "22";


}

void Host::list(const Command&& command) {
    (void)command;
    std::ifstream hosts_file("tiramisu.yaml");
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

void Host::test(const Command&& command)
{   
    if (command.arguments.size() == 0) {
        std::cerr << "tiramisu: error: <env_name> missing\n";
    }
    if (m_sshHandler->sshConnect()) {
        const auto alias = command.arguments.front();
        try {
            m_sshHandler->fillSshHandler(alias);
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        m_sshHandler->sshDisconnect();
    } else {
        throw std::runtime_error("remote host specs wrongs!");
    }
}

void Host::setup(const Command &&command)
{
    (void)command;
}

void Host::reset(const Command &&command)
{
    (void)command;
}

void Host::purge(const Command &&command)
{
    (void)command;
}