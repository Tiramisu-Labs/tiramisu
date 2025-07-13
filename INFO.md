Tiramisu CLI: Deployment Workflow and Technical Overview
The Tiramisu CLI leverages cross-compilation and SSH to deploy a lightweight, WASM-enabled server environment on a resource-constrained remote host.

1. The Local Machine (CLI Host)
The local machine is responsible for compilation, dependency management, and deployment.

CLI Function: tiramisu

Key Responsibilities:

Identify the remote host's architecture (e.g., ARM64, x86_64).

Cross-compile the custom C server.

Compile application .wasm files.

Transfer files securely (via SSH/SCP).

Configure and start services on the remote host.

Tools and Libraries
Cross-compilation Toolchain: You need the appropriate GCC toolchain for the target architecture (e.g., aarch64-linux-gnu-gcc for ARM Linux).

Wasmtime Library (Static): You must have the static library file (libwasmtime.a) compiled for the target architecture.

WASM Compilers: Tools to compile user applications to WASM (e.g., rustc with wasm32-wasi target, TinyGo, or Emscripten).

Cross-Compilation Process (C Server)
Retrieve Wasmtime Static Library: Ensure libwasmtime.a for the target architecture is available on the local machine.

Compile the C Server: Use the cross-compiler to build your C server, specifying static linking for Wasmtime and dynamic linking for system libraries (glibc).

Example Cross-Compilation Command:

Bash

# This command links Wasmtime statically (-Bstatic) and glibc dynamically (-Bdynamic)
<target-toolchain>-gcc -o custom_server custom_server.c \
  -I /path/to/wasmtime/include \
  -Wl,-Bstatic -lwasmtime -Wl,-Bdynamic \
  -L /path/to/wasmtime/lib \
  -ldl -lpthread # (Required for Wasmtime on Linux)
2. The Remote Host (Lightweight Server)
The remote host runs the services and executes WASM modules. The goal is to minimize installed dependencies on this machine.

Key Components:

Pre-compiled Nginx: Used as a reverse proxy.

Custom C Server: Your lightweight server, statically linked with Wasmtime.

WASM Modules (.wasm): The user applications.

Challenges and Solutions (No sudo required)
Challenge

Solution

Notes

Nginx Installation

Pre-compiled Nginx binaries distributed by the CLI.

Nginx requires root access for ports 80/443. Install it in a user directory (e.g., /home/user/tiramisu/nginx) and configure it to run on a high port (e.g., 8080).

Service Management

Use nohup or backgrounding scripts over SSH.

Since sudo is unavailable, systemd or init.d cannot be used directly. The CLI will start services as background processes.

Custom Server Execution

The cross-compiled binary is placed in a user-accessible directory and executed directly.

Due to static linking of Wasmtime, no additional dependencies are needed for execution.


Esporta in Fogli
Deployment Steps via Tiramisu CLI
The CLI will orchestrate the deployment using SSH and SCP (Secure Copy Protocol).

A. Initial Setup (tiramisu setup)
Identify Target: The CLI determines the remote architecture.

Cross-Compile C Server: Compiles custom_server with static Wasmtime linking.

Prepare Nginx: Selects the appropriate pre-compiled Nginx binary and configuration files.

Transfer Files: Uses SCP to copy all files (Nginx, custom_server, configuration scripts) to a dedicated directory on the remote host (e.g., ~/tiramisu-env).

Configure and Start Services:

Uses SSH to configure Nginx to listen on a non-privileged port (e.g., 8080).

Uses SSH to launch Nginx and custom_server as background processes (nohup).

B. Application Deployment (tiramisu deploy <source>)
Compile to WASM: The CLI compiles the user application to a .wasm file using the appropriate WASM toolchain.

Transfer WASM: Uses SCP to upload the .wasm file to a designated location on the remote host (e.g., ~/tiramisu-env/wasm_modules/).

WASM Module Hot-Swap (Optional): The custom C server can be designed to monitor the directory or receive a signal to load the new .wasm module dynamically, minimizing downtime.