#include "../include/Lexer.hpp"
#include "../include/Token.hpp"

#include <cctype>
#include <regex>
#include <iostream>
#include <filesystem>

std::regex url_regex (
    R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
    std::regex::extended
  );

static bool isNumber(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(), 
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}


Lexer::~Lexer()
{
    tokens.clear();    
}

static bool isCommand(const std::string& command) {
    return command == "connect" ||
        command == "init" || command == "host" ||
        command == "create" || command == "build" ||
        command == "deploy" || command == "webserver" ||
        command == "install" || command == "setup" ||
        command == "help" || command == "unistall";
}

void Lexer::tokenize(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++) {
        std::string current = argv[i];
    
        size_t eq_pos = current.find('=');
        if (eq_pos != std::string::npos && current.rfind("-", 0) == 0) { 
            std::string option_name = current.substr(0, eq_pos);
            std::string option_value = current.substr(eq_pos + 1);
            tokens.push_back(Token(ETypes::OPTION_NAME, std::move(option_name), ""));
            tokens.push_back(Token(ETypes::OPTION_VALUE, std::move(option_value)));
            continue;
        }

        if (isCommand(current)) {
            tokens.push_back(Token(ETypes::COMMAND, std::move(current)));
        } else if (current.rfind("-", 0) == 0) {
            if (flags.count(current)) {
                tokens.push_back(Token(ETypes::FLAG, std::move(current), ""));
            } else if (options.count(current)) {
                tokens.push_back(Token(ETypes::OPTION_NAME, std::move(current), ""));
                if (i + 1 >= argc) {
                    throw std::runtime_error("Error: missing value for option " + current);
                }
                tokens.push_back(Token(ETypes::OPTION_VALUE, argv[++i]));
            } else {
                throw std::runtime_error("Error: unrecognized or malformed option/flag: " + current);
            }
        }
        else if (isNumber(current)) {
            tokens.push_back(Token(ETypes::NUMBER, std::move(current)));
        } else if (std::filesystem::exists(current)) {
            tokens.push_back(Token(ETypes::PATH, std::move(current)));
        } else {
            tokens.push_back(Token(ETypes::STRING, std::move(current)));
        }
    }
    tokens.push_back(Token(ETypes::EOF_TOKEN, ""));
}

Token Lexer::getNextToken()
{
    Token s {tokens.front()};
    tokens.pop_front();
    return s;
}

Token Lexer::peekNextToken()
{
    return tokens.front();
}

