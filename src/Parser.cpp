#include "../include/Parser.hpp"
#include "../include/Lexer.hpp"
#include "../include/Token.hpp"
#include <iostream>

Parser::Parser(std::unique_ptr<Lexer> lexer)
    : m_lexer(std::move(lexer)) {
    }

Token Parser::getNextToken() {
    return m_lexer->getNextToken();
}

Token Parser::peekNextToken() const {
    return m_lexer->peekNextToken();
}

void Parser::throwParserError(const std::string& message) const {
    throw std::runtime_error("Parser Error: " + message);
}

Command_t Parser::parse()
{
    Command_t command;
    
    Token current_token = getNextToken();

    std::cout << "current_token: " << current_token.getValue() << std::endl;

    if (current_token.getType() != ETypes::STRING) {
        throwParserError("Expected command name, but found: " + current_token.getValue());
    }
    
    command.name = current_token.getValue();
    if (command.name == "host") {
        command.subcommand = getNextToken().getValue();
    }
    while ((current_token = getNextToken()).getType() != ETypes::EOF_TOKEN) {
        std::cout << "current_token: " << current_token.getValue() << std::endl;
        switch (current_token.getType()) {
            case ETypes::FLAG: {
                command.flags.push_back(current_token.getMetadata());
                break;
            }
            case ETypes::OPTION_NAME: {
                std::string option_name = current_token.getValue();

                Token value_token = getNextToken();
                if (value_token.getType() != ETypes::OPTION_VALUE) {
                    throwParserError("Expected option value for '" + option_name + "', but found: " + value_token.getValue());
                }
                    command.options.push_back({option_name, value_token.getValue()});
                    break;
                }
            case ETypes::NUMBER: // fall through
            case ETypes::PATH:  // fall through
            case ETypes::STRING: {
                command.arguments.push_back(current_token.getValue());
                break;
            }
            default:
                throwParserError("Unexpected token type: " + current_token.getValue());
        }
    }
    return command;
}

