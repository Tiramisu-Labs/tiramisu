#include <project.hpp>

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
        


    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return std::nullopt;
    }
}