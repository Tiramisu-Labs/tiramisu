#pragma once

#include <deque>
#include <string>
#include <unordered_set>

#include "Utils.hpp"
#include "Token.hpp"

class Lexer
{
    private:
        int argc;
        std::deque<Token> tokens;
        const std::unordered_set<std::string> flags = {
            "--verbose", "-v",
            "--help", "-h",
            "--arch",
            "--no-guards",
            "--build"
        };
        const std::unordered_set<std::string> options = {
            "--output", "-o",
            "--dir", "-d",
            "--env", "-e",
            "--ip",
            "--user", "-u",
            "--key",
            "--port",
            "--password", "-p"
        };

    public:
        Lexer() = default;
        ~Lexer();

        Token getNextToken();
        Token peekNextToken();
        Token peekAhead(int n);

        void tokenize(int argc, char* argv[]);
};
