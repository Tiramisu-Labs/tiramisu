#pragma once

#include "Lexer.hpp"
#include "Utils.hpp"

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <iostream>

class Lexer;

class Parser
{
    private:
       std::unique_ptr<Lexer> m_lexer;
       std::deque<Token> m_lookahead_buffer;

       Token getNextToken();
       Token peekNextToken() const;

       void throwParserError(const std::string& message) const;
   public:
       Parser() = delete;
       Parser(std::unique_ptr<Lexer> lexer);

       Command_t parse();

    // utility
    
};
