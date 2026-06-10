#include <commands/Local.hpp>
#include <ClusterTemplates.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>

Local::Local() {}

Local::~Local() {}

std::string Local::getName() const { return "local"; }

std::string_view Local::getHelp() const { return LOCAL_HELP; }

void Local::execute(const Command& command)
{
    auto subcommand = command.arguments.front();
    try {
        commandsMap[subcommand](std::move(command));
    } catch (const std::runtime_error& e) {
        std::cerr << e.what();
    }

}

static bool is_docker_healthy() 
{
    #ifdef _WIN32
        int status = std::system("docker info >nul 2>&1");
    #else
        int status = std::system("docker info > /dev/null 2>&1");
    #endif

    return (status == 0);
}

void Local::start(const Command&& command)
{
    if (!is_docker_healthy()) {
        std::cerr << "error: Docker is either not installed or the daemon is not running.\n"
                  << "Please install Docker or start the Docker Desktop application and try again." 
                  << std::endl;
        return;
    }
    
    namespace fs = std::filesystem;
    std::string home_dir = std::getenv("HOME");
    fs::path cluster_path = fs::path(home_dir) / ".tiramisu" / "local-cluster";
    auto build_flag = std::find(command.flags.begin(), command.flags.end(), "--build");

    if (!fs::exists(cluster_path)) {
        std::error_code ec;
        fs::create_directories(cluster_path, ec);
        if (ec) {
            std::cerr << "error: failed to create directory " << cluster_path 
                      << " (" << ec.message() << ")" << std::endl;
            return;
        }
    }

    fs::path compose_path = cluster_path / "docker-compose.yaml";
    if (!fs::exists(fs::path(compose_path))) {
        std::ofstream out(compose_path);
        out << Tiramisu::Templates::DOCKER_COMPOSE;
        std::cout << " -> Unpacked docker-compose.yml\n";
    }

    if (!fs::exists(fs::path(cluster_path / "Dockerfile.runtime"))) {
        std::ofstream out(cluster_path / "Dockerfile.runtime");
        out << Tiramisu::Templates::DOCKERFILE;
        std::cout << " -> Unpacked Dockerfile.runtime\n";
    }

    std::string docker_cmd = "docker compose -f " + compose_path.string() + " up -d";

    if (build_flag != command.flags.end()) {
        docker_cmd += " --build";
        std::cout << "Container changes detected. Forcing clean image rebuild...\n";
    }

    std::cout << "Booting local virtual cloud environments (env-alpha & env-beta)...\n";

    // Execute the terminal process synchronously
    int status = std::system(docker_cmd.c_str());

    if (status != 0) {
        std::cerr << "error: Docker Compose failed to spin up the local cluster engine.\n"
                  << "Ensure the Docker daemon is running on your host machine." << std::endl;
        return;
    }

    std::cout << "\nLocal Cloud Mesh Environment successfully deployed!\n"
              << " -> env-alpha targets | HTTP: localhost:8082 | SSH: localhost:2221 (Pass: tiramisu_local_dev)\n"
              << " -> env-beta  targets | HTTP: localhost:8083 | SSH: localhost:2222 (Pass: tiramisu_local_dev)\n\n"
              << "Run 'tiramisu host add <env_name> --ip 127.0.0.1 --port 2221' to provision env-alpha.\n";
}

void Local::stop(const Command&& command)
{
    (void)command;

    std::cout << "Stopping local containers (env-alpha & env-beta)...\n";

    int status = std::system("docker compose -f ~/.tiramisu/local-cluster/docker-compose.yaml stop");

    if (status != 0) {
        std::cerr << "error: Docker Compose failed to stop local cluster engine.\n";
        return;
    }

    std::cout << "\nLocal Cloud Mesh Environment successfully stopped!\n";
}

void Local::status(const Command&& command)
{
    (void)command;

    if (!is_docker_healthy()) {
        std::cerr << "error: Docker is either not installed or the daemon is not running.\n"
                  << "Please start Docker Desktop and try again." << std::endl;
        return;
    }

    namespace fs = std::filesystem;
    std::string home_dir = std::getenv("HOME");
    fs::path cluster_path = fs::path(home_dir) / ".tiramisu" / "local-cluster";
    fs::path compose_path = cluster_path / "docker-compose.yaml";

    if (!fs::exists(compose_path)) {
        std::cout << "Local cluster environment has not been initialized yet.\n"
                  << "Run 'tiramisu local start' to unpack configurations and boot the mesh.\n";
        return;
    }

    std::cout << "\n========================================================================\n";
    std::cout << "                 TIRAMISU LOCAL ENVIRONMENT STATUS MATRIX               \n";
    std::cout << "========================================================================\n";

    std::string docker_cmd = "docker compose -f " + compose_path.string() + " ps";
    int status = std::system(docker_cmd.c_str());

    if (status != 0) {
        std::cerr << "error: Failed to extract runtime statistics from Docker Compose.\n";
        return;
    }

    std::cout << "========================================================================\n";
    std::cout << "Local Access Mapping Matrix:\n"
              << "  [Alpha Environment] -> HTTP: http://localhost:8082 | SSH: localhost:2221\n"
              << "  [Beta Environment]  -> HTTP: http://localhost:8083 | SSH: localhost:2222\n\n"
              << "Next Steps:\n"
              << "  To provision Alpha: 'tiramisu host add <env_name> --ip 127.0.0.1 --port 2221'\n"
              << "  To deploy a function on added hosts: 'tiramisu deploy [local_path]'\n";
    std::cout << "========================================================================\n\n";
}

void Local::clean(const Command&& command)
{
    auto options = command.options;

    namespace fs = std::filesystem;
    std::string home_dir = std::getenv("HOME");
    fs::path cluster_path = fs::path(home_dir) / ".tiramisu" / "local-cluster";
    fs::path compose_path = cluster_path / "docker-compose.yaml";

    if (!fs::exists(cluster_path)) {
        std::cout << "Your local system is already a pristine canvas. Nothing to clean!\n";
        return;
    }

    auto yes_flag = std::find_if(command.flags.begin(), command.flags.end(),
        [&](const std::string& s){return s == "-y" || s == "--yes"; }
    );

    if (yes_flag == command.flags.end()) {
        std::cout << "  WARNING: This will permanently destroy all local container nodes,\n"
                  << "   wipe out all test Postgres databases, and erase deployed binaries.\n"
                  << "   Are you absolutely sure you want to proceed? [y/N]: ";
        
        std::string response;
        std::getline(std::cin, response);
        if (response != "y" && response != "Y") {
            std::cout << "Clean operation aborted safely.\n";
            return;
        }
    }

    if (fs::exists(compose_path)) {
        if (!is_docker_healthy()) {
            std::cerr << "error: Docker daemon is dead. Cannot safely scrub internal database volumes.\n"
                      << "Please launch Docker Desktop to let Tiramisu purge runtime resources." << std::endl;
            return;
        }

        std::cout << "Removing container instances and purging isolated volumes...\n";
        std::string docker_cmd = "docker compose -f " + compose_path.string() + " down -v";
        int status = std::system(docker_cmd.c_str());
        if (status != 0) {
            std::cerr << "error: failed to clean local environment\n";
        }
    }

    std::error_code ec;
    fs::remove_all(cluster_path, ec);
    
    if (ec) {
        std::cerr << "Infrastructure dropped, but failed to clear configuration directory: " 
                  << ec.message() << "\n";
    } else {
        std::cout << "Local cluster footprints erased. System is back to a completely blank canvas!\n";
    }
}