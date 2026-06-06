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
        std::cerr << "❌ error: Docker is either not installed or the daemon is not running.\n"
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

    std::cout << "\n🎉 Local Cloud Mesh Environment successfully deployed!\n"
              << " -> env-alpha targets | HTTP: localhost:8081 | SSH: localhost:2221 (Pass: tiramisu_local_dev)\n"
              << " -> env-beta  targets | HTTP: localhost:8082 | SSH: localhost:2222 (Pass: tiramisu_local_dev)\n\n"
              << "Run 'tiramisu host setup --host 127.0.0.1 --port 2221' to provision env-alpha.\n";
}

void Local::stop(const Command&& command)
{
    (void)command;
}

void Local::status(const Command&& command)
{
    (void)command;
}

void Local::clean(const Command&& command)
{
    auto options = command.options;
}