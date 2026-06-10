#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <filesystem>
#include <optional>
#include <iostream>

struct Environment {
    std::string host;
    std::string user = "root";
    int port = 22;
    std::filesystem::path key = "~/.ssh/id_rsa";
};

class Project {
    private:
    std::string name;
    std::unordered_map<std::string, Environment>    envs;
    std::filesystem::path configPath;
    // private construct to ensure the object is created using loadFromContext
    Project(const std::string& n, const std::unordered_map<std::string, Environment> envs, std::filesystem::path& path);

    public:
    // delete constructor to force using loadFromContext initializer
    Project() = delete;
    
    std::optional<Environment> getEnv(const std::string& env_name) const;
    static std::optional<Project> loadFromContext(std::filesystem::path start_dir);
    
    void addOrUpdateEnv(const std::string& env_name, const Environment& env);
    void print() const;
    bool save() const;
};