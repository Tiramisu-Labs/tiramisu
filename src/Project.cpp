#include <Project.hpp>
#include <Yaml.hpp>
#include <commands/ICommand.hpp>

Project::Project(
    std::string n,
    std::string d,
    std::unordered_map<std::string, Environment> environments,
    std::filesystem::path path
)
    : name(n), domain(d), envs(std::move(environments)), configPath(path) {};


std::optional<Environment> Project::getEnv(const std::string& env_name) const {
    auto it = envs.find(env_name);
    if (it != envs.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::unique_ptr<Project> Project::loadFromContext(std::filesystem::path start_dir) {
    std::filesystem::path config_path;
    
    while (true) {
        std::filesystem::path potential_config = start_dir / "tiramisu.yaml";
        if (std::filesystem::exists(potential_config)) {
            config_path = potential_config;
            break;
        }
        if (start_dir == start_dir.parent_path()) {
            std::cerr << "Error: tiramisu.yaml not found.\n";
            return nullptr;
        }
        start_dir = start_dir.parent_path();
    }

    try {
        YAML yaml;
        auto root_yaml = yaml.parseYaml(config_path); 

        std::string project_name = "Untitled";
        if (root_yaml["project"]["name"].isString()) {
            project_name = root_yaml["project"]["name"].as<std::string>();
        }

        std::string domain = "localhost";
        if (root_yaml["project"]["domain"].isString()) {
            domain = root_yaml["project"]["domain"].as<std::string>();
        }

        std::unordered_map<std::string, Environment> envs;

        for (const auto& [env_key, env_node] : root_yaml["environments"]) {
            if (!env_node.isNode()) continue;
            
            Environment env;
            if (env_node["host"].isString())      env.host = env_node["host"].as<std::string>();
            if (env_node["user"].isString())      env.user = env_node["user"].as<std::string>();
            if (env_node["key"].isString())       env.key  = env_node["key"].as<std::string>();
            if (env_node["arch"].isString())      env.arch  = env_node["arch"].as<std::string>();
            
            if (env_node["port"].isNumber()) {
                env.port = env_node["port"].as<int>(); 
            }

            envs[env_key] = std::move(env);
        }

        return std::unique_ptr<Project>(new Project(project_name, domain, std::move(envs), config_path));

    } catch (const std::exception& e) {
        std::cerr << "Parser error: " << e.what() << std::endl;
        return nullptr;
    }
}
void Project::addOrUpdateEnv(const std::string& env_name, const Environment& env) {
    envs[env_name] = env;
}

void Project::print() const
{
    std::cout << "project: \n  " << name << ":\n\n";
    std::cout << "environments: \n";
    for (const auto& [key, value] : envs) {
        std::cout << "  " << key << ":\n";
        std::cout << "    host: " << value.host << "\n"
                  << "    user: " << value.user << "\n"
                  << "    key: " << value.key << "\n"
                  << "    port: " << value.port << "\n"
                  << "    arch: " << value.arch << "\n";
    }
}

bool Project::save() const {
    std::ofstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "error: could not open config file for saving: " << configPath << "\n";
        return false;
    }

    file << "project:\n  name: \"" << name << "\"\n\n";
    file << "environments:\n";
    
    for (const auto& [env_name, env] : envs) {
        file << "  " << env_name << ":\n";
        file << "    host: " << env.host << "\n";
        file << "    user: " << env.user << "\n";
        file << "    port: " << env.port << "\n";
        if (!env.key.empty()) {
            file << "    key: " << env.key << "\n";
        }
        file << "    arch: " << env.arch << "\n";
    }    
    return true;
}

std::unique_ptr<Project> Project::getProject(const Command& command)
{
    auto dir = command.options.find("--dir");
    std::filesystem::path path = dir != command.options.end()
                            ? std::filesystem::path(dir->second)
                            : std::filesystem::current_path();

    auto project = Project::loadFromContext(path);
    if (!project) {
        throw std::runtime_error("error: tiramisu.yaml not found. Try using --dir <path> and specify a path where looking for it\n");
    }
    return project;
}