#include <commands/Init.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <memory>

Init::Init() {};

std::string Init::getName() const { return "init"; }

std::string Init::getHelp() const {
    return "Usage: tiramisu init <project_name> [options]"
"Initialize a clean, language-agnostic Tiramisu project workspace."

"This command sets up a polyglot API development tree. Tiramisu does not force "
"you into a single language ecosystem. The resulting directory structure natively "
"accepts and mixes C, C++, JavaScript, Python, Rust, and Zig source files simultaneously. "

"Caffeine (the execution engine) routes incoming HTTP requests dynamically by matching "
"filesystem paths to on-the-fly compiled shared objects (.so) using binary-masked chunk "
"headers."

"Arguments:"
"  <project_name>      The name of the root directory to create for your project."
"                      Must be alphanumeric and URL-safe."

"Options:"
"  -h, --help          Show this help message and exit."
"  --prefix <string>   Override the default incoming URL namespace prefix "
                      "(Default: 'api/v1')."

"Generated Structure:"
"  <project_name>/"
"  ├── tiramisu.yaml   # Global orchestration, routing, and SSH target configurations."
"  ├── api/            # The multilingual route root. Drop any supported language source file here to map it to an automatic API route."
"  └── static/         # Here you can create a static site to be uploaded inside the host"

"Examples:"
"  tiramisu init my_baremetal_api"
  
  "# After initialization, your workflow accepts mixed environments instantly:"
  "cd my_baremetal_api"
  "echo '...' > api/compute.c   -> Maps to /api/v1/compute (Native speed)"
  "echo '...' > api/users.js     -> Maps to /api/v1/users   (QuickJS shim)"
  "echo '...' > api/notify.py    -> Maps to /api/v1/notify  (Cython shim)";
}

static inline void initProject(const std::string& project_name, const std::string& api_path) {
    std::filesystem::create_directory(project_name);
    std::filesystem::create_directory(project_name + "/api");
    std::filesystem::create_directory(project_name + "/static");

    std::ofstream outfile(project_name + "/tiramisu.yaml");
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not create tiramisu.yaml\n";
        return;
    }

    outfile << "# Tiramisu Project Configuration\n"
            << "project:\n"
            << "  name: \"" << project_name << "\"\n\n"
            << "routing:\n"
            << "  prefix: \"" << api_path << "\"\n"
            << "  root_dir: \"./api\"\n\n"
            << "environments:\n"
            << "  # Use 'tiramisu host add' to append target server nodes here\n";

    outfile.close();

    std::cout << "project " + project_name + " succesfully created\n";
}

void Init::execute(const Command_t& command) {
    if (command.arguments.size() < 1) throw std::runtime_error(getHelp());
    
    auto const project_name = command.arguments.front();
    std::string default_api_path = "api/v1";
    // std::cout << "Init\n";
    // auto const sub_command = command.arguments[1];

    // if (sub_command == "-h" || sub_command == "--help") {
    //     getHelp();
    //     return;
    // } else if (sub_command == "--prefix") {
    //     try {
    //         default_api_path = command.arguments.at(2);
    //     } catch (const std::out_of_range& e) {
    //         std::cout << "error: " << e.what() << std::endl;
    //     }
    // }

    initProject(project_name, default_api_path);
}