#pragma once
#include "ICommand.hpp"
#include <string_view>


inline constexpr std::string_view CREATE_HELP = R"(
    Usage: tiramisu create <route_path> --lang <language> [options]
    
    Scaffold a new endpoint function template at a designated API route path.
    
    This command automatically generates the target directories and boilerplate code for 
    the language of your choice. Because Tiramisu relies on filesystem-based routing,
    the nested file path you provide translates directly into your live public API URL.
  
    Arguments:
      <route_path>        The URL route path where the function should live.
                          Do not include file extensions (e.g., 'users/profile').
   
    Options:
      -l, --lang <string> The programming language template to generate. *(Required)*
                          Supported: c, cpp, js (javascript), py (python), rust, zig.
      -h, --help          Show this help message and exit."
    
    Behavior:
      The CLI scans your local 'tiramisu.yaml' config to find your function routing root,
      creates any missing subdirectories, and appends the appropriate file extension
      automatically based on the chosen language.
    
    Examples:
      # Creates functions/analytics/compute.c
      tiramisu create analytics/compute --lang c      -> Maps to /api/v1/analytics/compute
    
      # Creates functions/users/get.js
      tiramisu create users/get --lang js             -> Maps to /api/v1/users/get
    
      # Creates functions/auth/verify.py
      tiramisu create auth/verify --lang py           -> Maps to /api/v1/auth/verify
)";

class Create : public ICommand
{
private:
public:
    Create();
    ~Create() override {};

    std::string getName() const override;
    std::string_view getHelp() const override;

    void execute(const Command& command) override;
};
