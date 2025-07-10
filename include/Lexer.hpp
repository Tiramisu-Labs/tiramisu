#pragma once

#include <deque>
#include <string>
#include <unordered_set>

#include "Utils.hpp"
#include "Token.hpp"

class Lexer
{
    private:
        int m_argc;
        std::deque<Token> m_tokens;
        const std::unordered_set<std::string> m_boolean_flags = {"--verbose", "-v", "--help", "-h"};
        const std::unordered_set<std::string> m_options_with_value_next_arg = {"--output", "-o"};

    public:
        Lexer() = default;
        ~Lexer();

        Token getNextToken();
        Token peekNextToken();
        Token peekAhead(int n);

        void tokenize(int argc, char* argv[]);
};
