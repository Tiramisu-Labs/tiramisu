#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <filesystem>
#include <algorithm>

enum class ETypes
{
    COMMAND,
    STRING,
    NUMBER,
    ADDRESS,
    PATH,
    FLAG,
    OPTION_NAME,
    OPTION_VALUE,
    EOF_TOKEN
};

struct Command {
    bool help = false;
    std::string name;
    std::unordered_map<std::string, std::string> options;
    std::vector<std::string> flags;
    std::vector<std::string> arguments;

    bool hasFlag(const std::string& flag) const {
        return std::find(flags.begin(), flags.end(), flag) != flags.end();
    }

    std::string getOption(const std::string& option, const std::string& fallback = "") const {
        auto it = options.find(option);
        if (it != options.end()) {
            return it->second;
        }
        return fallback;
    }

    const std::vector<std::string>& getArgs() const {
        return arguments;
    }
};
