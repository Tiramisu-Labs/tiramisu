#include <project.hpp>
#include <yaml.hpp>

Project::Project(const std::string& n, const std::unordered_map<std::string, Environment>&& environments)
    : name(n), envs(std::move(environments)) {};

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
        auto parsed_map = yaml.parseYaml(config_path);
        
        YAML root_yaml;
        for (auto& [k, v] : parsed_map) {
            root_yaml[k] = std::move(v);
        }

        std::string project_name = "Untitled";
        if (root_yaml["project"]["name"].isString()) {
            project_name = root_yaml["project"]["name"].as<std::string>();
        }

        std::unordered_map<std::string, Environment> envs;

        for (const auto& [env_key, env_node] : root_yaml["environments"]) {
            if (!env_node.isNode()) continue;
            
            Environment env;
            auto& mutable_node = const_cast<YAMLNode&>(env_node);

            if (mutable_node["host"].isString())     env.host = mutable_node["host"].as<std::string>();
            if (mutable_node["user"].isString())     env.user = mutable_node["user"].as<std::string>();
            if (mutable_node["identity"].isString()) env.key  = mutable_node["identity"].as<std::string>();
            if (mutable_node["port"].isNumber())     env.port = static_cast<int>(mutable_node["port"].as<double>());

            envs[env_key] = std::move(env);
        }

        return Project(project_name, std::move(envs));

    } catch (const std::exception& e) {
        std::cerr << "Parser error: " << e.what() << std::endl;
        return std::nullopt;
    }
}

void Project::print() const
{
    std::cout << "project: \n  " << name << ":\n\n";
    std::cout << "environments: \n";
    for (const auto& [key, value] : envs) {
        std::cout << "  " << key << ":\n";
        std::cout << "    host: " << value.host << "\n"
                  << "    user: " << value.user << "\n"
                  << "    identity: " << value.key << "\n"
                  << "    port: " << value.port << "\n";
    }
}