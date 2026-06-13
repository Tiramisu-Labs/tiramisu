#include <commands/Deploy.hpp>
#include <SshHandler.hpp>
#include <iostream>
#include <filesystem>
#include <vector>
#include <Project.hpp>
#include <SshHandler.hpp>

namespace fs = std::filesystem;
Deploy::Deploy() = default;
Deploy::~Deploy() = default;
std::string Deploy::getName() const { return "deploy"; }
std::string_view Deploy::getHelp() const {
    return DEPLOY_HELP;
}

void Deploy::execute(const Command& command) {
    if (command.help) {
        std::cout << getHelp();
        exit(0);
    }
    std::string path = "."; 
    bool dry_run = command.hasFlag("--dry-run");
    // bool clean_target = command.hasFlag("--clean-target");
    std::string env = command.getOption("--env", "");

    project = Project::getProject(command);

    if (!command.arguments.empty()) {
        for (const auto& arg : command.arguments) {
            deploy(arg, dry_run, env);
        }
    } else {
        deploy(path, dry_run, env);
    }
}

void Deploy::deploy(std::string path, bool dry_run, std::string env) {
    fs::path projectRoot = project->getConfigPath().parent_path();
    fs::path localSearchRoot = projectRoot / path;

    if (!fs::exists(localSearchRoot)) {
        std::cerr << "error: Local path target does not exist: " << localSearchRoot << "\n";
        return;
    }

    if (dry_run) {
        std::cout << "[DRY-RUN] Simulating workspace build sequence...\n";
    } else {
        std::cout << "Compiling application modules and native objects...\n";
        std::string cmd = "cd " + projectRoot.string() + " && tiramisu build " + path;
        if (std::system(cmd.c_str()) != 0) {
            std::cerr << "error: Compilation framework broken. Aborting deployment pipeline.\n";
            return;
        }
    }

    auto deployToTarget = [&](const Environment& env, const std::string& envName) {
        std::cout << "\n>>> Processing deployment for target environment: [" << envName << "]\n";
        
        SshHandler handler(env.host, env.user, env.port, env.key);
        if (!handler.connect()) {
            std::cerr << " [X] Error: Could not establish session connection with node " << envName << "\n";
            return;
        }

        std::string remotePrefix = "/var/lib/tiramisu/functions/" + project->getDomain() + "/";

        for (const auto& entry : fs::recursive_directory_iterator(localSearchRoot)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".so") {
                continue;
            }

            std::string localFilePath = entry.path().string();
            
            std::string archDirectoryToken = "/" + env.arch + "/";
            if (localFilePath.find(archDirectoryToken) == std::string::npos) {
                continue; 
            }

            fs::path relativePath = fs::relative(entry.path(), projectRoot);
            std::string relativePathStr = relativePath.string();

            std::string removeToken = env.arch + "/";
            size_t tokenPos = relativePathStr.find(removeToken);
            if (tokenPos != std::string::npos) {
                relativePathStr.erase(tokenPos, removeToken.length());
            }

            std::string remoteDestinationFile = remotePrefix + relativePathStr;

            if (dry_run) {
                std::cout << " [DRY-RUN] Target Match Discovered:\n"
                          << "   Source: " << localFilePath << "\n"
                          << "   Target: " << remoteDestinationFile << "\n";
                continue;
            }

            std::cout << " -> Uploading: " << relativePath.filename().string() << " to node filesystem...\n";
            if (handler.upload_to_dest(localFilePath, remoteDestinationFile)) {
                std::cout << "    [✓] Module successfully linked.\n";
            } else {
                std::cerr << "    [X] Sync failure tracking module payload mapping write error.\n";
            }
        }
    };

    if (!env.empty()) {
        auto getEnv = project->getEnv(env);
        if (!getEnv) {
            throw std::runtime_error("error: env name not found inside execution runtime profile layout (tiramisu.yaml)");
        }
        deployToTarget(*getEnv, env);
    } else {
        for (const auto& [name, targetEnv] : project->getEnvs()) {
            deployToTarget(targetEnv, name);
        }
    }
}