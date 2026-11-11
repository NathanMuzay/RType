# Installation

Every third-party library is installed by [vcpkg](https://vcpkg.io) in manifest mode, from `vcpkg.json`. You only
install a compiler, CMake, Git and vcpkg, plus a few system packages on Linux.

| Library | Used by | Purpose |
|---|---|---|
| SFML 3 | `engine_render`, `engine_audio`, `engine_input` (client) | Window, graphics, sound, keyboard, gamepad |
| Asio (standalone) | `engine_net` | UDP sockets |
| nlohmann-json | `engine_scene` | Reading the game files |
| pybind11 | `engine_scripting` | Embedding Python |
| Python 3 | `engine_scripting` (server and solo) | Running the game scripts |

Versions are fixed by the `builtin-baseline` of `vcpkg.json`, so every member builds with the same libraries.

## Linux (Ubuntu / Debian, WSL 2)

```sh
sudo apt update
sudo apt install build-essential cmake ninja-build git curl zip unzip tar pkg-config \
    libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev \
    autoconf autoconf-archive automake libtool
```

The autotools are only used by vcpkg to build Python. The first build compiles Python once, which takes several
minutes.

Install vcpkg inside the repository (the `.vcpkg/` folder is ignored by Git):

```sh
git clone https://github.com/microsoft/vcpkg.git .vcpkg
./.vcpkg/bootstrap-vcpkg.sh -disableMetrics
```

CMake detects `.vcpkg/` automatically. A vcpkg installed elsewhere also works, see [Building](building.md).

Under WSL 2, the windows open through WSLg on Windows 11. To play on a LAN from WSL, see
[Network play](network-play.md#4-wsl).

## Windows

1. Install **Visual Studio 2022** (or the Build Tools) with the **Desktop development with C++** workload.
2. Install **CMake 3.21+** and **Git**.
3. Install vcpkg, for example in `C:\vcpkg`, and set `VCPKG_ROOT`:

```powershell
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat -disableMetrics
setx VCPKG_ROOT C:\vcpkg
```

Open a new terminal after `setx` so that the variable is visible.

## Documentation website (optional)

```sh
pip install mkdocs-material
mkdocs serve
```

Then open `http://127.0.0.1:8000`.
