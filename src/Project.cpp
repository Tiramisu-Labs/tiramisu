#include <Project.hpp>
#include <Yaml.hpp>

Project::Project(
    const std::string& n,
    const std::unordered_map<std::string, Environment> environments,
    std::filesystem::path& path
)
    : name(n), envs(std::move(environments)), configPath(path) {};


std::optional<Environment> Project::getEnv(const std::string& env_name) const {
    auto it = envs.find(env_name);
    if (it != envs.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<Project> Project::loadFromContext(std::filesystem::path start_dir) {
    std::filesystem::path config_path;
    
    while (true) {
        std::filesystem::path potential_config = start_dir / "tiramisu.yaml";
        if (std::filesystem::exists(potential_config)) {
            config_path = potential_config;
            break;
        }
        if (start_dir == start_dir.parent_path()) {
            std::cerr << "Error: tiramisu.yaml not found.\n";
            return std::nullopt;
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

        std::unordered_map<std::string, Environment> envs;

        for (const auto& [env_key, env_node] : root_yaml["environments"]) {
            if (!env_node.isNode()) continue;
            
            Environment env;
            if (env_node["host"].isString())      env.host = env_node["host"].as<std::string>();
            if (env_node["user"].isString())      env.user = env_node["user"].as<std::string>();
            if (env_node["key"].isString())       env.key  = env_node["key"].as<std::string>();
            
            if (env_node["port"].isNumber()) {
                env.port = env_node["port"].as<int>(); 
            }

            envs[env_key] = std::move(env);
        }

        return Project(project_name, std::move(envs), config_path);

    } catch (const std::exception& e) {
        std::cerr << "Parser error: " << e.what() << std::endl;
        return std::nullopt;
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
                  << "    port: " << value.port << "\n";
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
    }    
    return true;
}