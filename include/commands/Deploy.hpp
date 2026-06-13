#pragma once
#include "ICommand.hpp"
#include <string>
#include <memory>

inline constexpr std::string_view DEPLOY_HELP = R"(
    Usage: deploy [local_path] [--dry-run] [--clean-target] [--env <string>]
        Compiles and uploads a specific target file, a selected directory slice, or the entire application.
        Maps the local relative path directly to the remote server filesystem.

        [local_path]: Optional. Points to the file or directory you want to push live immediately (defaults to '.').
        --dry-run: Simulates the compilation and displays exactly which remote file path will be written and what its resulting URL will be.
        --clean-target: When deploying a folder, drops any orphaned `.so` files on the remote server that no longer exist in your local source workspace.
        --env <env_name>: Deploy the shared object only on target env. If omitted, it will deploy on each env.
)";

class Deploy : public ICommand {
private:
    std::unique_ptr<class Project> project;

    // void upload(const std::string& dest);
public:
    Deploy();
    ~Deploy() override;

    void execute(const Command& command) override;
    std::string getName() const override;
    std::string_view getHelp() const override;

    void deploy(std::string path,
                bool dry_run = false,
                // bool clean_target = false,
                std::string env = "");
};