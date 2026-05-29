#pragma once

#include "ICommand.hpp"
#include <unordered_map>

inline constexpr std::string_view BUILD_HELP = R"(
"Usage: build <dir> [arguments...]"
"   Compile files inside dir into a shared object"
)";

enum class Extensions
{
    INVALID,
    C,
    CPP,
    RS,
    JS,
    TS,
    GO,
    PY,
    WASM
};

class Build : public ICommand {
    private:

    public:
    Build() = default;
    ~Build() override {};
    std::string getName() const override;
    std::string_view getHelp() const override;
    void execute(const Command& command) override;
    bool checkExtensionCompiler(const std::string ext);

    const std::unordered_map<std::string, Extensions> extensionsMap_ {
        {"c", Extensions::C},
        {"cpp", Extensions::CPP},
        {"rs", Extensions::RS},
        {"js", Extensions::JS},
        {"ts", Extensions::TS},
        {"go", Extensions::GO},
        {"py", Extensions::PY},
        {"wasm", Extensions::WASM},
    };
};
