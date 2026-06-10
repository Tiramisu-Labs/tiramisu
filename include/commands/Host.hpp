#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <memory>
#include <map>
#include <vector>
#include <functional>

#include "ICommand.hpp"
class SshHandler;

inline constexpr std::string_view HOST_HELP = R"(
    Usage: host <action> [arguments...]
        Manages host configurations.
        host setup [--skip-nginx] [--caffeine-version <string>]: Remotely provisions a clean OS into a fully functioning Tiramisu node.
        host reset [--keep-db] [-y, --yes]: Wipes out all deployed application functions while leaving the underlying Nginx, Caffeine, and database configurations intact.
        host purge [-y, --yes]: The destructive deep-clean. Uninstalls Nginx, deletes the Caffeine binary, wipes all systemd services, and flushes all storage folders.
        host add <env_name> [--ip <string>] [--user <string>] [--password <string>] [--port <int>]: add a new host to the hosts list
        host ls list: list stored hosts
        host test <env_name>: test connection with the specified host
)";

inline constexpr std::string_view SETUP_SCRIPT = R"SCRIPT(#!/bin/bash
# ==============================================================================
# Tiramisu Platform - Remote Host Provisioning Script (Universal Mode)
# ==============================================================================
set -euo pipefail

# --- PRIVILEGE DETECTION ---
if [ "$(id -u)" -eq 0 ]; then
    SUDO=""
elif command -v sudo >/dev/null 2>&1; then
    SUDO="sudo"
else
    echo "Error: This script requires root privileges, but sudo is not installed." >&2
    exit 1
fi

# --- CONFIGURATION LAYER ---
REPO_URL="https://github.com/Tiramisu-Labs/caffeine.git" 
BUILD_DIR="/tmp/caffeine_compile"
CAFFEINE_ARGS="--port 80"

echo "Starting Caffeine provisioning..."

# 1. Install System Dependencies
echo "Step 1: Installing build tools..."
$SUDO apt-get update && $SUDO apt-get install -y gcc make git

# 2. Create System Infrastructure
echo "Step 2: Creating platform directories..."
$SUDO mkdir -p /var/log/tiramisu/
$SUDO mkdir -p /var/lib/tiramisu/functions/

# 3. Download and Build Caffeine Natively
echo "Step 3: Cloning and compiling Caffeine source..."
$SUDO rm -rf "$BUILD_DIR"
git clone "$REPO_URL" "$BUILD_DIR"
cd "$BUILD_DIR"

# Compile using the Makefile
make clean
make DEBUG=0

# 4. Install Binary via Makefile
echo "Step 4: Installing binary via Makefile..."
$SUDO make install

# 5 & 6. Process Management Layer (Systemd vs Standalone Background)
if [ -d /run/systemd/system ]; then
    echo "Step 5: Systemd detected. Registering service..."
    $SUDO tee /etc/systemd/system/caffeine.service > /dev/null << EOF
[Unit]
Description=Caffeine Standalone Server Runtime Engine
After=network.target

[Service]
Type=simple
ExecStart=/usr/local/bin/caffeine ${CAFFEINE_ARGS}
Restart=always
User=root

[Install]
WantedBy=multi-user.target
EOF

    echo "Step 6: Launching service via systemd..."
    $SUDO systemctl daemon-reload
    $SUDO systemctl enable --now caffeine
else
    echo "Step 5: Systemd NOT detected (Container/WSL environment). Skipping service registration..."
    
    echo "Step 6: Launching Caffeine as a background daemon process..."
    # Kill any existing raw instances to prevent port binding conflicts
    $SUDO killall caffeine 2>/dev/null || true
    
    # Run in background, redirect output to the proper log directory, detach from SSH terminal
    $SUDO nohup /usr/local/bin/caffeine ${CAFFEINE_ARGS} >> /var/log/tiramisu/caffeine.log 2>&1 &
    
    echo "Process spawned in background. Logs routing to /var/log/tiramisu/caffeine.log"
fi

# 7. Cleanup Build Garbage
echo "Step 7: Cleaning up temporary compilation files..."
$SUDO rm -rf "$BUILD_DIR"

echo "Caffeine successfully provisioned and running!"
)SCRIPT";


class Host : public ICommand {
    private:
    std::string getArch(const std::string& dir = "", const std::string& env = "") const;
    // factory method
    void add(const Command&& command);
    void list(const Command&& command);
    void setup(const Command&& command);
    void reset(const Command&& command);
    void purge(const Command&& command);
    void test(const Command&& command);

    public:
    Host() = default;
    ~Host() override {};

    std::string getName() const override;
    std::string_view getHelp() const override;
    void execute(const Command& command) override;

    std::map<std::string, std::function<void(const Command&& command)>> commandsMap = {
        {"add",   [this](const Command&& cmd) { add(std::move(cmd)); }},
        {"list",  [this](const Command&& cmd) { list(std::move(cmd)); }},
        {"ls",    [this](const Command&& cmd) { list(std::move(cmd)); }},
        {"setup", [this](const Command&& cmd) { setup(std::move(cmd)); }},
        {"reset", [this](const Command&& cmd) { reset(std::move(cmd)); }},
        {"purge", [this](const Command&& cmd) { purge(std::move(cmd)); }},
        {"test",  [this](const Command&& cmd) { test(std::move(cmd)); }},
    };
};
