#pragma once

#include "ICommand.hpp"
#include <unordered_map>

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
    Build();
    ~Build() override {};
    std::string getName() const override;
    std::string getHelp() const override;
    void execute(const Command_t& command) override;
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