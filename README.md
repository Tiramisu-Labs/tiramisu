# Tiramisu CLI
Tiramisu is a lightweight CLI designed to manage a private cloud/serverless environment on resource-constrained hosts. It leverages WASM (WebAssembly) for secure, efficient execution on minimal infrastructure.

Getting Started
1. Host Management
Tiramisu requires registering remote hosts via SSH. Once registered, you can manage them using an alias.

Bash

## Register a new host (e.g., dev-server at user@192.168.1.100)
tiramisu host add <alias> <username@hostname_or_ip>

## List all registered hosts
tiramisu host list
2. Setup and Installation
The tiramisu setup command prepares the remote host. It automatically identifies the host architecture, cross-compiles the custom C server (statically linked with Wasmtime), installs pre-compiled Nginx binaries, and transfers all files to the remote host.

Bash

## Install the Tiramisu environment on the specified host
tiramisu setup <alias>
3. Application Lifecycle
Tiramisu simplifies the process of building and deploying WASM applications.

Building WASM Applications
The tiramisu build command compiles application source code into a WASM executable (.wasm).

Bash

## Compile source files recursively into WASM executables
tiramisu build <source_directory_or_file>
Deploying WASM Applications
The tiramisu deploy command transfers the compiled WASM files to the remote host.

Bash

## Deploy WASM executables and associated files to the remote host
tiramisu deploy <alias> <local_file_or_directory>
Webserver Management (tiramisu webserv)
The webserv command group manages the custom C server and Nginx on the remote host.

Service Control
Bash

## Install the web service binaries and configuration files on the remote host
## (Note: The WASM runtime is compiled directly into the web service binary)
tiramisu webserv install <alias> 

## Start the web service (custom server and Nginx)
tiramisu webserv start <alias>

## Stop the web service
tiramisu webserv shutdown <alias>

## Restart the web service
tiramisu webserv restart <alias>

## Reload the web service configuration (e.g., Nginx config)
tiramisu webserv reload <alias>
Configuration and Status
Bash

## Check the status of the web service
tiramisu webserv status <alias>

## Display logs for the web service
tiramisu webserv logs <alias>

## Test the current configuration files before applying them
tiramisu webserv test-config <alias>

## Open a configuration editor or apply changes to the web service configuration
tiramisu webserv configure <alias>
Connection and Uninstallation
Bash

## Establish an SSH connection to the web service host
tiramisu webserv connect <alias>

## Uninstall the web service from the remote host
tiramisu webserv uninstall <alias>

emcc: emcc -sSTANDALONE_WASM=1 -sPURE_WASI=1 file.c/cpp -o file.wasm


TODO:
    - environment var load
    - parse/update config.yaml
    - install wasmtime remotely
