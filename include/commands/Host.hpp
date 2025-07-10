#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <memory>
#include <map>
#include <vector>

#include "ICommand.hpp"
#include "Utils.hpp"

class Host : public ICommand {
    private:
    Command_t command;

    void add(std::vector<std::pair<std::string, std::string>> options);
    void list();
    std::map<std::string, std::string> getHostSpec(std::string& alias);

    public:
    Host();
    // override
    ~Host() override {};
    std::string getName() const override;
    std::string getHelp() const override;
    void execute(const Command_t& command) override;

    enum class Commands {
        INVALID,
        ADD,
        LIST,
        EXPORT,
        IMPORT,
        DELETE,
        SIZE
    };

    std::map<std::string, Commands> host_cmds = {
        {"add", Commands::ADD},
        {"list", Commands::LIST},
        {"export", Commands::EXPORT},
        {"import", Commands::IMPORT},
        {"delete", Commands::DELETE},
    };

    
};