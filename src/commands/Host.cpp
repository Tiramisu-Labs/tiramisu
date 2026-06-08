#include <commands/Host.hpp>
#include <SshHandler.hpp>
#include <project.hpp>

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
    std::cout << "sub_command: " << sub_command << std::endl;    
    commandsMap[sub_command](std::move(command));
}

void Host::add(const Command&& command) {
    if (command.arguments.empty()) {
        std::cerr << "error: an env name is required.\n\n" << getHelp();
        return ;
    }
    const auto env_name = command.arguments[0];
    if (!command.options.contains("--ip")) {
        std::cerr << "tiramisu: error: an ip is required.\n" << getHelp() << std::endl;
        return;
    }
    const std::string ip = command.options.find("--ip")->second;
    const std::string user = command.options.contains("--user") ? command.options.find("--user")->second : "root";
    const std::string key = command.options.contains("--key") ? command.options.find("--key")->second : "~/.ssh/id_rsa";
    const std::string port = command.options.contains("--port") ? command.options.find("--port")->second : "22";

    // try to enstablish and ssh connection
    std::cout << "Probing target node: " << user << "@" << ip << ":" << port << "...\n";
    // SshHandler handler = SshHandler();
    // bool isConnect = handler.sshConnect(ip, user, std::atoi(port.c_str()));

}

void Host::list(const Command&& command) {
    #if DEBUG
        std::cout << "host list\n";
    #endif
    auto dir = command.options.find("--dir");
    std::filesystem::path path = dir != command.options.end()
                                ? std::filesystem::path(dir->second)
                                : std::filesystem::current_path();
    if (auto project_opt = Project::loadFromContext(path)) {        
        Project project = std::move(*project_opt);
        project.print();
    } else {
        std::cerr << "Could not initialize project configuration.\n";
    }
}

std::string Host::getArch(const Command&& command) const {
    #if DEBUG
        std::cout << "host --arhc\n";
    #endif
    std::string arch = "";
    auto dir = command.options.find("--dir");
    std::filesystem::path path = dir != command.options.end()
                                ? std::filesystem::path(dir->second)
                                : std::filesystem::current_path();

    if (auto project_opt = Project::loadFromContext(path)) {
        Project project = std::move(*project_opt);
        auto env = command.options.find("--env");
        if (env != command.options.end()) {
            if (auto getEnv = project.getEnv(env->second)) {
                SshHandler handler = SshHandler(getEnv->host, getEnv->user, getEnv->port);
                arch = handler.getArch();
            }
        } else {
            throw std::runtime_error("error: environment name missing. Use --env <env_name>");
        }
    } else {
        std::cerr << "Could not initialize project configuration.\n";
    }
    return arch; 
}

void Host::test(const Command&& command)
{   
    (void)command;
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