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
    m_tokens.clear();    
}

void Lexer::tokenize(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++) {
        std::string current = argv[i];
    
        size_t eq_pos = current.find('=');
        if (eq_pos != std::string::npos && current.rfind("-", 0) == 0) { 
            std::string option_name = current.substr(0, eq_pos);
            std::string option_value = current.substr(eq_pos + 1);
            m_tokens.push_back(Token(ETypes::OPTION_NAME, std::move(option_name), ""));
            m_tokens.push_back(Token(ETypes::OPTION_VALUE, std::move(option_value)));
            continue;
        }

        if (current.rfind("-", 0) == 0) {
            if (m_boolean_flags.count(current)) {
                m_tokens.push_back(Token(ETypes::FLAG, std::move(current), ""));
            }
            else if (m_options_with_value_next_arg.count(current)) {
                m_tokens.push_back(Token(ETypes::OPTION_NAME, std::move(current), ""));
                if (i + 1 >= argc) {
                    throw std::runtime_error("Error: missing value for option " + current);
                }
                m_tokens.push_back(Token(ETypes::OPTION_VALUE, argv[++i]));
            } else {
                throw std::runtime_error("Error: unrecognized or malformed option/flag: " + current);
            }
        }
        else if (isNumber(current)) {
            m_tokens.push_back(Token(ETypes::NUMBER, std::move(current)));
        } else if (std::filesystem::exists(current)) {
            m_tokens.push_back(Token(ETypes::PATH, std::move(current)));
        } else {
            m_tokens.push_back(Token(ETypes::STRING, std::move(current)));
        }
    }
    m_tokens.push_back(Token(ETypes::EOF_TOKEN, ""));
}

Token Lexer::getNextToken()
{
    Token s {m_tokens.front()};
    m_tokens.pop_front();
    return s;
}

Token Lexer::peekNextToken()
{
    return m_tokens.front();
}

