#include <iostream>
#include <string>
#include "../include/CLI.hpp"
#include "../include/Parser.hpp"
#include "../include/Lexer.hpp"

#include <fstream>
#include <filesystem>

int main(int argc, char *argv[])
{
    auto lexer = std::make_unique<Lexer>();

    lexer->tokenize(argc, argv);

    auto parser = std::make_unique<Parser>(std::move(lexer));

    CLI cli = CLI(std::move(parser), "./");

    cli.run();
}
