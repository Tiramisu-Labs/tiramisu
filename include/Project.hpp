#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <filesystem>
#include <optional>
#include <iostream>
#include <memory>

struct Environment {
    std::string             host;
    std::string             user = "root";
    int port =              22;
    std::filesystem::path   key = "~/.ssh/id_rsa";
    std::string             arch;
};

class Project {
    private:
    std::string name;
    std::string domain;
    std::unordered_map<std::string, Environment>    envs;
    std::filesystem::path configPath;
    // private construct to ensure the object is created using loadFromContext
    Project(std::string n, std::string d, std::unordered_map<std::string, Environment> envs, std::filesystem::path path);

    public:
    // delete constructor to force using loadFromContext initializer
    Project() = delete;
    
    std::optional<Environment> getEnv(const std::string& env_name) const;
    inline std::unordered_map<std::string, Environment> getEnvs() const { return envs; }
    static std::unique_ptr<Project> loadFromContext(std::filesystem::path start_dir);
    
    void addOrUpdateEnv(const std::string& env_name, const Environment& env);
    void print() const;
    bool save() const;

    static std::unique_ptr<Project> getProject(const class Command& command);
    
    inline std::filesystem::path getConfigPath() const { return configPath; }
    inline std::string getDomain() const { return domain; }
};