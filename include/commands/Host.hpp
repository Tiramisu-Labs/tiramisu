#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <memory>
#include <map>
#include <vector>

#include "ICommand.hpp"
// #include "Utils.hpp"
class SshHandler;

class Host : public ICommand {
    private:
    std::unique_ptr<SshHandler> m_sshHandler;

    void add(std::unordered_map<std::string, std::string> options);
    void list();
    void test(const std::unordered_map<std::string, std::string>& options);
    void test();
    std::string getArch() const;

    public:
    Host();
    Host(std::unique_ptr<SshHandler>&& handler);
    // override
    ~Host() override {};
    std::string getName() const override;
    std::string getHelp() const override;
    void execute(const Command_t& command) override;

    enum class Commands {
        INVALID,
        HELP,
        ADD,
        LIST,
        EXPORT,
        IMPORT,
        DELETE,
        TEST,
        SIZE
    };

    std::map<std::string, Commands> host_cmds = {
        {"add", Commands::ADD},
        {"list", Commands::LIST},
        {"ls", Commands::LIST},
        {"export", Commands::EXPORT},
        {"import", Commands::IMPORT},
        {"delete", Commands::DELETE},
        {"test", Commands::TEST},
    };

    
};