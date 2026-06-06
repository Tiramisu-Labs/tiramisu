## Global Context Flags

These parameters can be supplied to any command. They configure how your local C++ CLI communicates with the single-host server via SSH/SFTP and how it scopes routes.

| Flag | Type | Default | Description |
| --- | --- | --- | --- |
| `-h, --host` | String | *Required* | Remote host IP or hostname (e.g., `192.168.1.50`). |
| `-u, --user` | String | `root` | SSH execution username. |
| `-i, --identity` | String | `~/.ssh/id_rsa` | Path to the local SSH private key file. |
| `-p, --port` | Integer | `22` | Remote SSH daemon port. |
| `--prefix` | String | `api/v1` | The global base route prefix for API paths. |
| `--verbose` | Boolean | `false` | Outputs raw shell executions and SFTP transfer logs. |

---

## 1. Remote Host & Infrastructure Control (`tiramisu host ...`)

These commands automate setting up, wiping, or auditing the single server's bare-metal runtime environment via remote SSH scripting.

## 1. `tiramisu host add <env_name>`

Links a new target physical hardware node to your local project space, executes interactive security credential handshakes, and appends the target definition to `tiramisu.yaml`.

* **Arguments:** `<env_name>` (e.g., `production`, `staging`, `lab-pi`).
* **Flags & Options:**
* `--ip <string>`: The target server IPv4 address or hostname. *(Required)*
* `--user <string>`: SSH connection username. *(Default: `root`)*
* `--key <path>`: Local path to the private SSH key to use for daily tasks. *(Default: `~/.ssh/id_rsa`)*
* `--port <int>`: Target SSH daemon listening port. *(Default: `22`)*



### Under-the-Hood Execution Flow & Security Bootstrap:

1. **Agnostic Key Probe:** The CLI instantly attempts an implicit, non-interactive SSH connection to the provided `--ip` using the specified private `--key`.
2. **Pass-through Success:** If the connection succeeds, the remote host is already authorized. The CLI skips to step 5.
3. **One-Time Password Prompt:** If the connection fails with a public-key rejection error, the terminal halts execution and prompts the user securely:
```text
[!] SSH key not authorized on remote host.
[?] Enter password for user '@192.168.1.50': 

```


4. **Automated Security Provisioning:** The CLI uses your SSH class to log in with the password *exactly once*. It reads the local public key matching the user's flag choice (e.g., parsing `~/.ssh/id_rsa.pub` if `--key` was `~/.ssh/id_rsa`) and executes this exact atomic permission string on the remote machine:

```bash
    mkdir -p ~/.ssh && chmod 700 ~/.ssh && echo 'ssh-rsa AAAAB3N...' >> ~/.ssh/authorized_keys && chmod 600 ~/.ssh/authorized_keys
```
5.  **Final Validation:** The CLI drops the password from memory, closes the session, and re-runs a clean probe execution using *only* the private key. If this second check passes, it appends the block cleanly to `tiramisu.yaml`:
    
```yaml
    environments:
      production:
        host: "192.168.1.50"
        user: "root"
        identity: "~/.ssh/id_rsa"
        port: 22
```

---

## 2. `tiramisu host setup [env_name]`
Completely provisions a verified bare-metal node into a running Tiramisu platform. This command assumes SSH keys are authorized and runs entirely under the hood.

*   **Arguments:** `[env_name]` (Optional. Scopes target. Defaults to the first block inside `tiramisu.yaml` if omitted).
*   **Flags & Options:**
    *   `--skip-nginx`: Standard text installation skips binding the web routing engine.
*   **Under-the-Hood Execution Flow:**
    The CLI connects via your key-based SSH class and drops a cross-compiled sequence of setup bash operations:
    1.  Validates remote system architecture (`uname -m`) to ensure it targets your low-end platform constraints.
    2.  Installs Nginx and creates your uniform reverse proxy configuration mapping `/api/v1/*` to the local internal Unix Domain Socket file tree.
    3.  Pushes your native, language-agnostic Caffeine binary directly into `/usr/local/bin/caffeine`.
    4.  Generates the standard directory infrastructure: creates `/var/lib/tiramisu/functions/` to act as the target destination for your file-system `.so` router.
    5.  Registers and launches the Caffeine server runtime inside systemd.

---

## 3. `tiramisu host reset [env_name]`
Wipes out all functional application state from the remote single-board computer without altering base infrastructure systems or standalone local data services.

*   **Arguments:** `[env_name]` (Optional).
*   **Flags & Options:**
    *   `-y, --yes`: Suppresses warning verifications.
*   **Under-the-Hood Execution Flow:**
    Connects silently using the target environmental key and clears the binary routing playground. It executes:
    
```bash
    rm -rf /var/lib/tiramisu/functions/*
```
    Because Caffeine uses on-the-fly disk checks via `stat()` to load chunk-masked binaries, your API layer is instantly scrubbed. Any incoming request from Nginx immediately yields an optimal, low-overhead 404 response directly from Caffeine.

---

## 4. `tiramisu host purge [env_name]`
The ultimate destructive uninstallation sequence. Completely tears down the server layer, scrubbing the system back to its native state.

*   **Arguments:** `[env_name]` (Optional).
*   **Flags & Options:**
    *   `-y, --yes`: Directly forces bypass of terminal security locks.
*   **Under-the-Hood Execution Flow:**
    Logs in with your key and purges all operational configurations:
    1.  Stops and deletes the Caffeine systemd service.
    2.  Removes Nginx upstream server block definitions.
    3.  Deletes `/usr/local/bin/caffeine` and recursively drops `/var/lib/tiramisu/`.

---

## Summary Summary of Command UX

| Command | Daily Auth Method | Interactive Prompts | Primary Objective |
| :--- | :--- | :--- | :--- |
| `tiramisu host add` | Password (Once, to inject key) | Only if key check fails | Registers a node; forces strict `authorized_keys` file privileges. |
| `tiramisu host setup` | SSH Private Key (Under the hood) | None | Provisions core directories, sets up Nginx socket, starts Caffeine. |
| `tiramisu host reset` | SSH Private Key (Under the hood) | Confirmation Prompt | Drops all `.so` binaries for an instant runtime wipe. |
| `tiramisu host purge` | SSH Private Key (Under the hood) | Confirmation Prompt | Total uninstallation of Caffeine and application files. |

---

## 2. Granular Development, Compilation & Deployment

Because Caffeine maps URLs directly to disk paths, these commands handle compiling individual targets and matching that directory layout exactly on the remote machine.

### `tiramisu init`

Scaffolds a local project folder structure with proper low-level C-ABI boilerplate templates.

* **Flags & Options:**
* `--name <string>`: Name string for your root project directory. *(Required)*
* `--lang <c | rust | zig | js | python>`: Setup wrapper template to inject.



### `tiramisu build [local_path]`

Invokes local, containerized toolchains to cross-compile source files into optimized `.so` binaries matching the remote host's architecture.

* **Arguments:** `[local_path]` (Optional. Can be a single file like `utils/to_string.c` or an entire subdirectory like `auth/`. If omitted, the CLI builds everything under your local directory).
* **Flags & Options:**
* `--release`: Compiles with heavy optimizations (`-O3`, strips debug map symbols) to minimize binary size.
* `--cc-flags <string>`: Passes raw tracking flags directly into the backend compiler.



### `tiramisu deploy [local_path]`

Compiles and uploads a specific target file, a selected directory slice, or the entire application. It maps the local relative path directly to the remote server filesystem.

* **Arguments:** `[local_path]` (Optional. Points to the file or directory you want to push live immediately).
* **Flags & Options:**
* `--dry-run`: Simulates the compilation and displays exactly which remote file path will be written and what its resulting URL will be.
* `--clean-target`: When deploying a folder, drops any orphaned `.so` files on the remote server that no longer exist in your local source workspace.


* **Path Resolution Mechanics:**
* If you run `tiramisu deploy utils/to_string.c`, the CLI compiles it, creates the folder `/var/lib/tiramisu/functions/utils/` on the remote server via SFTP if it doesn't exist, and uploads `to_string.so`.
* **Resulting URL:** `http://<host>/api/v1/utils/to_string` (Caffeine reads this path, evaluates `/var/lib/tiramisu/functions/utils/to_string.so`, loads it, and serves).



---

## 3. Local Stateful Dependencies (`tiramisu svc ...`)

Since you are operating on a single host, databases are installed directly onto the host's bare metal (ideally directed to a mounted, high-performance external SSD).

### `tiramisu svc install <postgres | sqlite3 | redis>`

Installs non-containerized data dependency engines onto the host operating system.

* **Flags & Options:**
* `--storage-path <path>`: Directs where the raw database storage files should live (e.g., `/mnt/secure-ssd/postgres-data`).
* `--db-name / --db-user / --db-pass`: Setup administrative database credentials.
* `--port <int>`: Overrides standard network listening parameters.



### `tiramisu svc control <action>`

Wraps systemd interactions to safely change database execution states.

* **Arguments:** `<action>` must be one of: `start`, `stop`, `restart`, `status`.
* **Flags & Options:**
* `--name <string>`: Targets a specific data engine dependency instance (e.g., `postgres`).



### `tiramisu svc uninstall <service_name>`

Completely purges a stateful engine from the host OS.

* **Flags & Options:**
* `-y, --yes`: Suppresses warning prompts.



---

## 4. Native Runtime Service Management (`tiramisu service ...`)

Maps standard system execution states directly to your remote core infrastructure components over SSH.

```text
tiramisu service <start | stop | restart | reload | status> [options]

```

* **Flags & Options:**
* `-t, --target <caffeine | nginx | all>`: Specifies which layer of the core server infrastructure to command. (Defaults to `all`).
* *Note: Because Caffeine resolves your `.so` files completely on-the-fly from disk via `stat()`, you do not need to call `reload` when adding new endpoints! You only need to use `reload` or `restart` if you update Caffeine's own internal settings or configuration properties.*



---

## 5. System Observability & Diagnostics (`tiramisu sys ...`)

Provides visibility into the resource-constrained host without requiring manual terminal emulation tools.

### `tiramisu sys logs`

Streams unified telemetry sequences directly back to your local C++ CLI workspace output.

* **Flags & Options:**
* `-f, --follow`: Toggles persistent real-time streaming updates (`tail -f`).
* `-n, --lines <int>`: Sets the historical trailing lines depth window (Defaults to `50`).
* `--target <caffeine | nginx | system>`: Filters logs down to a specific subsystem layer.



### `tiramisu sys top`

Renders a live terminal status dashboard of the physical machine.

* Displays real-time CPU core loads and physical board operating temperatures.
* Displays total available RAM utilization.
* Lists the absolute virtual memory addresses where active `.so` handler blocks are currently loaded into memory space by Caffeine's workers.

### `tiramisu sys df`

Queries active storage metrics and remaining gigabytes on the server’s Micro-SD card and attached external SSD arrays.

---

## Quick Example Scenarios

### Scenario A: Deploying a single fast C utility function

```bash
tiramisu deploy utils/to_string.c --host 192.168.1.50
# Compiles locally, transfers to remote host, creates remote directory layout.
# URL is live instantly: http://192.168.1.50/api/v1/utils/to_string

```

### Scenario B: Testing a whole new API folder section with verbose output

```bash
tiramisu deploy billing/ --host 192.168.1.50 --release --verbose
# Walks billing/ folder, compiles all files inside, syncs them up to /var/lib/tiramisu/functions/billing/

```

---

## 6. Local Virtualized Cloud Orchestration (`tiramisu local ...`)

These commands manage your global, shared local testing playground. It abstracts Docker Compose to spin up isolated container nodes (`env-alpha` and `env-beta`) directly on your machine, mimicking your real Raspberry Pi edge nodes without polluting your host OS.

### `tiramisu local start`

Initializes and runs the background containerized infrastructure stack on your development machine.

* **Flags & Options:**
* `--build`: Forces Docker to rebuild the runtime images from scratch, updating any internal base configurations.


* **Under-the-Hood Execution Flow:**
1. **Self-Healing Probe:** Looks for the global directory `~/.tiramisu/local-cluster/`. If it is missing, the CLI creates the path and materializes the embedded `docker-compose.yml`, `nginx.conf`, and `Dockerfile.runtime` templates directly from the binary's internal memory string view.
2. **Stack Launch:** Spatially maps and boots the cluster silently in the background:
```bash
docker compose -f ~/.tiramisu/local-cluster/docker-compose.yml up -d

```


3. **Print Access Map:** Outputs a clean networking matrix showing where the developer can target deployments and send API requests locally.



---

### `tiramisu local stop`

Safely halts all locally running Tiramisu environment nodes and their stateful databases without deleting any data.

* **Under-the-Hood Execution Flow:**
Executes a direct stop command against the global tracking structure:
```bash
docker compose -f ~/.tiramisu/local-cluster/docker-compose.yml stop

```



---

### `tiramisu local status`

Renders an administrative breakdown of your local containers, verifying resource consumption and port mapping status.

* **Under-the-Hood Execution Flow:**
Queries Docker to see if `tiramisu-env-alpha` and `tiramisu-env-beta` are healthy, displaying which ports are actively listening on `localhost` (`2221`, `2222`, `8081`, `8082`).

---

### `tiramisu local clean`

Wipes out all local container states, dropping database volumes and erasing all deployed test `.so` files to return your local system back to a blank canvas.

* **Flags & Options:**
* `-y, --yes`: Directly forces bypass of terminal security locks.


* **Under-the-Hood Execution Flow:**
Tears down the containers and explicitly wipes out the persistent Docker volumes mapped to your local environments:
```bash
docker compose -f ~/.tiramisu/local-cluster/docker-compose.yml down -v

```

---

## Updated Quick Example Scenarios

Adding this command cleans up your testing workflows beautifully. You can append this new scenario to show off how the local cluster interacts with your existing deployment system:

### Scenario C: Testing multi-project environments entirely on your local machine

```bash
# 1. Boot up your local global cloud infrastructure once
tiramisu local start

# 2. Deploy your payment service to the local Alpha engine node
tiramisu deploy services/billing/ --host 127.0.0.1 --port 2221
# Live locally at: http://localhost:8081/api/v1/billing

# 3. Deploy your authentication service to the local Beta engine node
tiramisu deploy services/auth/ --host 127.0.0.1 --port 2222
# Live locally at: http://localhost:8082/api/v1/auth

```

---

How are you planning to parse the commands and arguments in your C++ CLI (`CLI.cpp`/`Parser.cpp`)? Are you writing a custom token loop, or are you utilizing a lightweight single-header library like `CLI11` to handle the subcommand nested structures?