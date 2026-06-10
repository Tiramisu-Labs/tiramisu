#include <commands/Host.hpp>
#include <SshHandler.hpp>
#include <Project.hpp>

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
    if (command.arguments.size() < 2) {
        std::cerr << "error: an env name is required.\n\n" << getHelp();
        return ;
    }

    const auto env_name = command.arguments[1];

    if (!command.options.contains("--ip")) {
        std::cerr << "tiramisu: error: an ip is required.\n" << getHelp() << std::endl;
        return;
    }
    auto dir = command.options.find("--dir");
    std::filesystem::path path = dir != command.options.end()
                                ? std::filesystem::path(dir->second)
                                : std::filesystem::current_path();

    auto project_opt = Project::loadFromContext(path);
    if (!project_opt) {
        std::cerr << "Could not find tiramisu.yaml. Try passing the specific path using --dir option\n";
        return;
    }

    if (project_opt->getEnv(env_name)) {
        std::cerr << "error: env_name already present. Please try using a different name\n";
        return;
    }

    Project project = std::move(*project_opt);

    std::filesystem::path home_env = std::getenv("HOME");
    
    const std::string ip = command.options.find("--ip")->second;
    const std::string user = command.options.contains("--user") ? command.options.find("--user")->second : "root";
    const std::string key = command.options.contains("--key") ? command.options.find("--key")->second : std::string(home_env / ".ssh/id_rsa.pub");
    const std::string port = command.options.contains("--port") ? command.options.find("--port")->second : "22";
    // const std::string password = command.options.contains("--password") ? command.options.find("--password")->second : "";

    // try to enstablish and ssh connection
    std::cout << "Probing target node: " << user << "@" << ip << ":" << port << "...\n";
    SshHandler handler = SshHandler(ip, user, std::atoi(port.c_str()), key);

    bool authenticated = handler.attemptConnection();

    if (!authenticated) {
        char* pass_ptr = getpass("Enter remote host password: ");
        if (pass_ptr) {
            std::string password(pass_ptr);
            authenticated = handler.try_password(password);
        }
    }

    if (authenticated) {
        if (handler.inject_local_public_key(key)) {
            Environment new_node{ ip, user, std::atoi(port.c_str()), key };                
            project.addOrUpdateEnv(env_name, new_node);
            
            if (project.save()) {
                std::cout << "Node successfully saved to configuration profile!\n";
            }
        }
    }
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

std::string Host::getArch(const std::string& dir, const std::string& env) const {
    #if DEBUG
        std::cout << "host --arhc\n";
    #endif
    std::string arch = "";
    std::filesystem::path path = dir != ""
                                ? std::filesystem::path(dir)
                                : std::filesystem::current_path();

    if (auto project_opt = Project::loadFromContext(path)) {
        Project project = std::move(*project_opt);
        if (env != "") {
            if (auto getEnv = project.getEnv(env)) {
                SshHandler handler = SshHandler(getEnv->host, getEnv->user, getEnv->port, getEnv->key);
                arch = handler.getArch();
            } else {
                throw std::runtime_error("error: environment name not found into tiramisu.yaml");    
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
    if (command.arguments.size() < 2) {
        std::cerr << "error: an env name is required.\n\n" << getHelp();
        return ;
    }

    const auto env_name = command.arguments[1];

    auto dir = command.options.find("--dir");
    std::filesystem::path path = dir != command.options.end()
                            ? std::filesystem::path(dir->second)
                            : std::filesystem::current_path();

    std::string arch = getArch(path, env_name);

    std::cout << "remote host arch: " << arch << std::endl;
}

void Host::setup(const Command &&command)
{
    if (command.arguments.size() < 2) {
        std::cerr << "error: an env name is required.\n\n" << getHelp();
        return ;
    }

    const auto env_name = command.arguments[1];

    auto dir = command.options.find("--dir");
    std::filesystem::path path = dir != command.options.end()
                            ? std::filesystem::path(dir->second)
                            : std::filesystem::current_path();

    // auto skip_nginx = std::find(command.flags.begin(), command.flags.end(), "--skip-nginx");
    // bool skip = skip_nginx != command.flags.end();
    // if (skip) {

    // }

    if (auto project_opt = Project::loadFromContext(path)) {
        Project project = std::move(*project_opt);
        if (env_name != "") {
            if (auto getEnv = project.getEnv(env_name)) {
                SshHandler handler = SshHandler(getEnv->host, getEnv->user, getEnv->port, getEnv->key);
                handler.exec_remote_command(std::string("bash -c '" + std::string(SETUP_SCRIPT) + "'"));
            } else {
                throw std::runtime_error("error: environment name not found into tiramisu.yaml");    
            }
        } else {
            throw std::runtime_error("error: environment name missing. Use --env <env_name>");
        }
    } else {
        std::cerr << "Could not initialize project configuration.\n";
    }

}

void Host::reset(const Command &&command)
{
    (void)command;
}

void Host::purge(const Command &&command)
{
    (void)command;
}