# Quick start: build and run everything

A practical, copy-paste guide from a fresh machine to a multiplayer game. Every command runs **from the root of the
repository**. The detailed pages are [Installation](installation.md), [Building](building.md),
[Running](running.md) and [Playing on a network](network-play.md).

## 1. What you get

| Executable | Role | Needs |
|---|---|---|
| `r-type_server` | R-Type server, no window | nothing |
| `r-type_client` | R-Type client: window, sound, gamepad | a running server |
| `skyhop` | The second game, solo | nothing |

Executables are in `build/bin/` on Linux and in `out\windows\bin\Release\` on Windows. The `data/`, `scripts/`,
`assets/`, `plugins/` and `python/` folders are copied next to them by the build: keep them.

Third-party libraries (SFML 3, Asio, nlohmann-json, pybind11, Python 3) are installed **automatically** by vcpkg from
`vcpkg.json`. Only the build tools are installed by hand.

## 2. Ubuntu / WSL

### Install the tools (once)
```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build git curl zip unzip tar pkg-config \
    libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev \
    autoconf autoconf-archive automake libtool
```
The autotools are used by vcpkg to build Python; the X11, OpenGL and udev packages are needed by SFML.

### Install vcpkg inside the repository (once)
```bash
git clone https://github.com/microsoft/vcpkg.git .vcpkg
./.vcpkg/bootstrap-vcpkg.sh -disableMetrics
```
`.vcpkg/` is ignored by Git and detected by CMake automatically.

### Configure (first time, or after deleting `build/`)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```
The first configuration downloads and builds every library, Python included: **expect 10 to 30 minutes**. Later
configurations are almost instant.

### Build
```bash
cmake --build build --parallel "$(nproc)"
```
The build succeeded when it ends with:
```text
[100%] Built target r-type_server
[100%] Built target r-type_client
[100%] Built target skyhop
```
!!! note
    Under WSL, `Clock skew detected` warnings are not errors: the WSL clock is slightly off. `sudo hwclock -s`
    removes them.

### Shortcut: build and start a server with two clients
```bash
./scripts/play.sh 2
```

## 3. Windows (PowerShell)

### Install the tools (once)
1. **Visual Studio 2022** (or the Build Tools) with the **Desktop development with C++** workload.
2. **CMake 3.21+** and **Git**.
3. **vcpkg**:
```powershell
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat -disableMetrics
setx VCPKG_ROOT C:\vcpkg
```
Close and reopen PowerShell after `setx`, then check that `echo $env:VCPKG_ROOT` prints `C:\vcpkg`.

### Configure (first time)
```powershell
cmake -S . -B out\windows -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
```

### Build
```powershell
cmake --build out\windows --config Release --parallel 4
```

### Run
```powershell
cd out\windows\bin\Release
.\r-type_server.exe -p 4242
```
In **another** PowerShell, from the same folder:
```powershell
.\r-type_client.exe -h 127.0.0.1 -p 4242
```
Skyhop: `.\skyhop.exe`, or a double-click.

## 4. Play

### R-Type on one machine
| Terminal | Ubuntu / WSL | PowerShell |
|---|---|---|
| 1: server | `./build/bin/r-type_server -p 4242` | `.\r-type_server.exe -p 4242` |
| 2: player 1 | `./build/bin/r-type_client -h 127.0.0.1 -p 4242` | `.\r-type_client.exe -h 127.0.0.1 -p 4242` |
| 3: player 2 | same command | same command |

In the game: type a **player name**, then **CONNECT**. One player chooses **CREATE ROOM** (mode COOP or VERSUS,
difficulty, rules), the others join it and can chat, and the owner presses **START GAME**.

### Command-line options
| Option | Program | Effect |
|---|---|---|
| `-p <port>` | server, client | UDP port, 4242 by default |
| `-h <address>` | client | server address; otherwise `serverAddress` from `rtype_settings.json` |
| `-g <game>` | all | load the data of another game (`rtype`, `skyhop`) |
| `--help` | all | print the usage |

### Controls
| Key | Action |
|---|---|
| Arrow keys | move |
| Space (hold to charge) | fire |
| Left Shift | launch or recall the Force |
| Escape | pause |
| F1 | console (`help` lists the commands; cheats need the room rule) |
| F3 | metrics: FPS, ping, entities, memory, bandwidth |

Keys, gamepad and volumes are set in the **SETTINGS** menu, or in `data/rtype_settings.json`.

## 5. Play on several machines

1. One machine starts the server; every player, the host included, starts a client.
2. Everybody uses **the same build** (same commit), otherwise the client reports a protocol version mismatch.
3. The machines are on **the same network**.
4. The server prints the address to use at start-up:
   ```text
   Server listening on UDP port 4242
   Players on your network can join with: -h 192.168.1.67 -p 4242
   ```
   Use this **local** address, never the public one shown by "what is my IP" websites. It changes from one
   network to another.
5. Open UDP port 4242 on the server machine:
   ```bash
   sudo ufw allow 4242/udp
   ```
   ```powershell
   # administrator PowerShell
   New-NetFirewallRule -DisplayName "R-Type UDP 4242" -Direction Inbound -Protocol UDP -LocalPort 4242 -Action Allow
   ```
6. A machine that only plays needs no compiler: copy the **whole** `out\windows\bin\Release\` folder (executables,
   DLLs, `assets`, `data`, `plugins`, `python`, `scripts`), for example as a zip:
   ```powershell
   Compress-Archive -Path "out\windows\bin\Release\*" -DestinationPath "$HOME\Desktop\RType_Windows.zip" -Force
   ```
   Extract it completely on the other machine, then run `.\r-type_client.exe -h <server address> -p 4242`.

A server inside WSL needs mirrored networking and a Hyper-V firewall rule, and some school networks isolate
machines: see [Playing on a network](network-play.md).

## 6. Useful commands

| Goal | Command |
|---|---|
| Build one target | `cmake --build build --target r-type_server` |
| Rebuild everything | `cmake --build build --clean-first` |
| Start from scratch (runs vcpkg again) | `rm -rf build && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release` |
| Dedicated server without SFML | `cmake -S . -B build-server -DCMAKE_BUILD_TYPE=Release -DENGINE_BUILD_CLIENT=OFF` |
| Protocol probe with 30 % loss | `python3 scripts/protocol_probe.py 4242 0.3` |
| Load test, 16 games | `python3 scripts/load_test.py 4242 16` |

### Check the formatting like the CI
Install the same `clang-format` version as the CI once, then check:
```bash
sudo apt install -y pipx && pipx install clang-format==18.1.8 && pipx ensurepath
find engine plugins -name '*.cpp' -o -name '*.hpp' -o -name '*.tpp' | xargs clang-format --style=file --dry-run --Werror
```
No output means the code is correctly formatted. Replace `--dry-run --Werror` with `-i` to fix it.

### Documentation website
```bash
python3 -m venv .venv && . .venv/bin/activate
pip install mkdocs-material
mkdocs serve
```

## 7. Common problems

| Message or symptom | Cause | Fix |
|---|---|---|
| `CMAKE_TOOLCHAIN_FILE does not exist` | vcpkg missing or `VCPKG_ROOT` empty | install vcpkg (sections 2 or 3), delete the build folder, configure again |
| `Could not find a package configuration file provided by "SFML"` | configured without the vcpkg toolchain | delete the build folder and configure again |
| `Clock skew detected` (WSL) | WSL clock slightly off | harmless; `sudo hwclock -s` |
| Client: protocol version mismatch | client and server from different builds | rebuild both from the same commit |
| Client: connection timed out, the server never answered | wrong address, firewall, different networks | section 5; try `-h 127.0.0.1` on the server machine first |
| `DataNode: missing key ...` at start-up | a required key is missing in a JSON file | add it, see [Game file](../reference/game-file.md) |
| The Windows client does not start on another machine | DLLs or folders missing | copy the whole `Release\` folder |

Exit codes: `0` on a normal exit, `84` on an error, with the message printed in the terminal. More cases in
[Troubleshooting](troubleshooting.md).
