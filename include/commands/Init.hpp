#pragma once

#include "ICommand.hpp"

inline constexpr std::string_view INIT_HELP = R"(
    "Usage: tiramisu init <project_name> [options]"
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
  'echo '...' > api/compute.c   -> Maps to /api/v1/compute (Native speed)'
  'echo '...' > api/users.js     -> Maps to /api/v1/users   (QuickJS shim)'
  'echo '...' > api/notify.py    -> Maps to /api/v1/notify  (Cython shim)'
)";

class Init : public ICommand {
    private:

    public:
    Init() = default;
    ~Init() override {};

    std::string getName() const override;
    std::string_view getHelp() const override;

    void execute(const Command& command) override;

};
