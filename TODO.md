This is a comprehensive overview of the Tiramisu CLI and its core functionalities. To ensure everything works smoothly, here is a detailed technical TODO list covering the main development phases.

Tiramisu CLI: Development TODO List
Phase 1: Local Machine Setup and Toolchain
Goal: Establish the cross-compilation environment and acquire necessary binaries.

[ ] Identify Target Architectures: Determine the specific Linux architectures (e.g., x86_64-linux-gnu, aarch64-linux-gnu) Tiramisu will support.

[ ] Install Cross-Compiler Toolchains: Install the required GCC toolchains on the local machine for all target architectures.

Example (Debian/Ubuntu for ARM64): sudo apt install gcc-aarch64-linux-gnu

[ ] Compile Wasmtime for Static Linking:

Clone the Wasmtime repository.

Cross-compile Wasmtime for each target architecture, ensuring the output includes the static library file (libwasmtime.a).

[ ] Acquire Nginx Binaries: Download or compile Nginx binaries for each target architecture. Ensure they are built to run without root privileges (if necessary, configure them to run on ports >1024).

Phase 2: Custom C Server Development
Goal: Create the lightweight web server and ensure it integrates correctly with Wasmtime.

[ ] Implement C Server with Wasmtime C API: Write the core server logic using standard C networking libraries (e.g., poll, epoll).

[ ] Integrate Wasmtime: Use the Wasmtime C API to load, instantiate, and execute WASM modules.

[ ] Implement WASM Module Management:

Design a mechanism for the C server to discover and manage WASM modules deployed to the remote host.

Implement an efficient way to reload or replace WASM modules without restarting the entire server.

Phase 3: CLI Implementation (Local Machine)
Goal: Automate the setup, compilation, and deployment processes.

[ ] Host Management (tiramisu host):

Develop functions to store and retrieve SSH credentials and host information.

[ ] Target Identification: Implement logic to connect via SSH and identify the remote host's architecture.

[ ] Cross-Compilation Logic (tiramisu setup):

Develop a build script that selects the correct cross-compiler and static libwasmtime.a based on the remote host's architecture.

Implement the -Wl,-Bstatic and -Wl,-Bdynamic flags to link Wasmtime statically while linking glibc dynamically.

[ ] Deployment Automation (tiramisu setup, tiramisu webserv install):

Use SCP to transfer binaries (Nginx, C server) and configuration files.

Use SSH to set permissions and create necessary directories on the remote host.

[ ] Service Management (tiramisu webserv):

Implement SSH commands to start, stop, and restart services in the background (e.g., using nohup).

[ ] WASM Build and Deploy (tiramisu build, tiramisu deploy):

Integrate WASM compilation commands (e.g., rustc --target wasm32-wasi).

Implement SCP for transferring .wasm files to the remote host.