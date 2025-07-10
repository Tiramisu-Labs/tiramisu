#pragma once

#include <string>
#include <vector>

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

struct Command_t {
    std::string name;
    std::string subcommand;
    std::vector<std::pair<std::string, std::string> > options;
    std::vector<std::string> flags;
    std::vector<std::string> arguments;
};
