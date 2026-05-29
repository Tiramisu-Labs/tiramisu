#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <filesystem>

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
};
