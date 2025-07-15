#include "../../include/commands/Build.hpp"

#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <stdlib.h>

Build::Build() {}

std::string Build::getName() const { return "build"; }

std::string Build::getHelp() const {
    return "Usage: build <dir> [arguments...]\n"
        "  Compile files inside dir into .wasm executable.\n";
}

void Build::execute(const Command_t& command) {
    std::cout << "execute build\n";
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
            auto it = extensionsMap_.find(extension);
            switch (it != extensionsMap_.end() ? it->second : Extensions::INVALID) {
                case Extensions::INVALID: {
                    std::cerr << "Error: extension unrecognized: ." << extension << "\n";
                    break;
                }
                case Extensions::C: {
                    if (!checkExtensionCompiler(extension)) {
                        std::string error = "no compiler found to compile .c files into .wasm\n"
                        "Please visit https://emscripten.org/ and follow the guide to install it,\n"
                        "or try running 'tiramisu install emscripten' to try automatically install it.";
                        throw std::runtime_error(error);
                    }
                    std::string cmd = "emcc -sSTANDALONE_WASM=1 -sPURE_WASI=1 " + file + " -o " + file.substr(0, file.find_last_of('.')) + ".wasm";
                    std::cout << cmd << "\n";
                    int s_call = system(cmd.c_str());
                    std::cout << s_call << std::endl;
                    break;
                }
                case Extensions::WASM: continue;
                default: break;
            }
        }
    } else if (std::filesystem::is_regular_file(path, ec)) {
    }
}
bool Build::checkExtensionCompiler(const std::string ext)
{
    int status = 0;
    if (ext == "c") {
        #ifdef _WIN32
            status = system("Get-Package -Name emscripten");
        #else
            status = !system("dpkg -l emscripten");
        #endif
    }
    return status;
}