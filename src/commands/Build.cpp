#include "../../include/commands/Build.hpp"

#include <stdexcept>
#include <iostream>
#include <filesystem>

Build::Build() {}

std::string Build::getName() const { return "build"; }

std::string Build::getHelp() const {
    return "Usage: build <dir> [arguments...]\n"
        "  Compile files inside dir into .wasm executable.\n";
}

void Build::execute(const Command_t& command) {
    std::string dir = "";
    std::string host = "";
    try {
        dir = command.arguments.at(0);
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: " << e.what() << "\n";
        getHelp();
        return ;
    }

    const std::filesystem::path path(dir);
    std::error_code ec;
    if (std::filesystem::is_directory(path, ec)) {
        for (const auto & entry : std::filesystem::directory_iterator(path)) {
            std::cout << entry.path() << std::endl;
            std::string file = entry.path();
            std::string extension = file.substr(file.find_last_of('.') + 1);
            if (extension == "wasm") {
                continue;
            }
            std::cout << extension << std::endl;
            auto it = extensionsMap_.find(extension);
            switch (it != extensionsMap_.end() ? it->second : Extensions::INVALID)
            {
            case Extensions::C: {
                std::string cmd = "emcc -sSTANDALONE_WASM=1 -sPURE_WASI=1 " + file + " -o " + file.substr(0, file.find_last_of('.')) + ".wasm";
                std::cout << cmd << "\n";
                int s_call = system(cmd.c_str());
                std::cout << s_call << std::endl;
                break;
            }
            default:
                break;
            }
        }
    } else if (std::filesystem::is_regular_file(path, ec)) {
        
    }
}