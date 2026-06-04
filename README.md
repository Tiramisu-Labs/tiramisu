## Tiramisu: Private Cloud & Serverless Engine

Tiramisu is a high-performance, lightweight serverless infrastructure designed for resource-constrained hardware like the Raspberry Pi. It replaces heavy containerization and WASM runtimes with a native Dynamic Loader model, executing code at bare-metal speeds via Shared Objects (.so).

---

## System Architecture

The project consists of three primary layers:

1. **Orchestrator (Tiramisu CLI):** Manages remote hosts via SSH. It handles the local cross-compilation of user code into architecture-specific Shared Objects and automates deployment.
2. **Edge Gateway (Nginx):** Acts as the entry point. It serves static assets (SSG) and proxies dynamic API requests to the Caffeine server via Unix Domain Sockets.
3. **Execution Engine (Caffeine Server):** A C-based server that utilizes a pre-forked worker pool and `SO_REUSEPORT`. It dynamically loads (`dlopen`) handler binaries into its memory space, executing logic through a unified C-ABI.

---

## Core Components

### The Caffeine Server

* **Listener:** Uses `epoll` and `SO_REUSEPORT` to handle up to 15,000 concurrent connections.
* **Worker Pool:** Persistent processes that maintain a cache of loaded `.so` handles to eliminate cold-start latency.
* **Supervisor:** Monitors worker health, memory usage, and execution time (Watchdog), recycling processes that leak or hang.
* **Context Injection:** Passes a `tiramisu_ctx` struct to handlers, providing pre-opened database handles (SQLite/Postgres) and request metadata.

### The Tiramisu CLI

* **Build Engine:** Uses containerized toolchains to cross-compile source code (C, Rust, Zig) into `.so` files without requiring local compiler installation.
* **Language Shims:** Wraps high-level languages like JavaScript (QuickJS) or Python (libpython) into C-compatible shared libraries.
* **Deployer:** Synchronizes binaries and updates Nginx/Caffeine configurations on remote hosts.

---

## Project Structure

```text
tiramisu/
├── bin/                # Compiled CLI binaries
├── cmd/                # Tiramisu CLI source code
├── server/             # Caffeine Execution Engine (C)
│   ├── src/            # Core server logic (epoll, workers, loader)
│   └── include/        # Server-internal headers
├── libs/               # Developer SDKs
│   ├── c/              # tiramisu.h (Context and API definitions)
│   ├── rust/           # tiramisu-rs (C-ABI bindings)
│   └── js/             # QuickJS wrapper templates
├── templates/          # Build-time wrappers for code generation
└── docker/             # Cross-compilation toolchain images

```

---

## Developer Workflow

1. **Define:** User writes a simple function using the Tiramisu library.
2. **Configure:** A `tiramisu.yaml` defines routes, database requirements, and static asset paths.
3. **Build:** `tiramisu build` compiles the code into a native `.so` for the target architecture.
4. **Deploy:** `tiramisu deploy` pushes the binary and static files to the remote host.
5. **Execute:** Caffeine detects the new file, hot-swaps the library pointer, and begins serving requests with zero downtime.

---

## Installation

Tiramisu can be compiled from source and installed globally. The build process places the CLI binary into your system path, while environment profiles remain completely isolated to standard user permissions.

### 1. Install System Dependencies

Ensure your host machine has a compiler supporting C++20, standard development tools, and `libssh` installed.

* **Ubuntu/Debian:**
```bash
sudo apt install build-essential c++ libssh-dev

```


* **macOS:**
```bash
brew install libssh

```



### 2. Compile and Install Globally

Clone the source repository, compile the project, and link it directly to your global environment stream:

```bash
git clone https://github.com/your-username/tiramisu.git
cd tiramisu
make && sudo make install

```

* `make` triggers localized compiler optimizations (`-O3`) and compiles the native C++ binary.
* `sudo make install` maps the compiled executable into `/usr/local/bin/tiramisu`.

### 3. Initialize the Local Cloud Environment

Tiramisu does not pollute your distinct project directories with complex configurations. Instead, it relies on a shared, global environment infrastructure.

To initialize and boot your local testing nodes (`env-alpha` and `env-beta`), run:

```bash
tiramisu local start

```

> **Note:** On first launch, the CLI automatically detects if your workspace is missing config mappings. It will safely unpack its embedded Docker Compose stack, custom Nginx configurations, and standard initialization scripts into `~/.tiramisu/local-cluster/` under your normal user privileges before starting the environments.

---

## Technical Specifications

* **Concurrency Model:** Multi-process Shared Listener (Linux SO_REUSEPORT).
* **Communication:** Unix Domain Sockets (Nginx to Caffeine).
* **Interface:** C-ABI / `#[repr(C)]`.
* **Memory Management:** Process-level isolation via Worker recycling and Watchdog.
* **Persistence Support:** Integrated SQLite (file-based) or PostgreSQL (connection pooling).

---

## Build Requirements

* **Local Machine:** Docker (for cross-compilation engines), SSH client.
* **Remote Host:** Linux (Kernel 3.9+ for SO_REUSEPORT), Nginx, Glibc.

This architecture ensures that even the smallest ARM devices can function as robust cloud nodes, prioritizing high-concurrency and minimal administrative overhead.