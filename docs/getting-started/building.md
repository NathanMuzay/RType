# Building

The first configuration downloads and builds the libraries of `vcpkg.json` (several minutes). Later builds reuse
them.

## Linux

From the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel "$(nproc)"
```

With vcpkg installed outside the repository:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --parallel "$(nproc)"
```

Output: `build/bin/`.

## Windows (PowerShell)

```powershell
cmake -S . -B out\windows -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
cmake --build out\windows --config Release --parallel 4
```

Output: `out\windows\bin\Release\`. The `out/` folder is ignored by Git.
`out\windows\RType.sln` can be opened in Visual Studio.

## What the build produces

```text
bin/
├── r-type_server     R-Type server
├── r-type_client     R-Type client
├── skyhop            Skyhop, solo
├── data/             game files of every game
├── scripts/          Python scripts
├── assets/           images, sounds, music, fonts
└── python/           Python standard library used by the embedded interpreter
```

`data/`, `scripts/` and `assets/` are copied from the game folders at every build. After editing a JSON or
Python file, run the build again to copy it.

## Targets

| Target | Kind | Content |
|---|---|---|
| `engine_core`, `engine_physics`, `engine_ecs`, `engine_scene` | libraries | Base engine |
| `engine_net`, `engine_network` | libraries | Transport and protocol, server and client connection |
| `engine_render`, `engine_audio`, `engine_input`, `engine_client` | libraries | Client side |
| `engine_scripting`, `engine_simulation` | libraries | Server side |
| `engine_local` | library | Solo mode |
| `engine_python_runtime` | custom | Copies the Python standard library |
| `rtype_content`, `skyhop_content` | custom | Copy each game's data, scripts and assets |
| `r-type_server`, `r-type_client`, `skyhop` | executables | Generic launchers built for each game |

Build one target only: `cmake --build build --target r-type_server`.

## Options

| Option | Default | Effect |
|---|---|---|
| `CMAKE_BUILD_TYPE` (Linux) / `--config` (Windows) | - | `Release` for playing, `Debug` for debugging |
| `ENGINE_BUILD_CLIENT` | `ON` | Builds the client modules and executables (needs SFML) |
| `ENGINE_BUILD_SERVER` | `ON` | Builds the server modules and executables (needs Python) |

A server-only build for a dedicated machine, without SFML:

```sh
cmake -S . -B build-server -DENGINE_BUILD_CLIENT=OFF
cmake --build build-server
```

The solo executables are only built when both options are `ON`.

## Clean build

```sh
rm -rf build          # Linux
rmdir /s /q out       # Windows
```

Do it after moving files, switching to a very different branch, or changing the vcpkg location. Problems:
[Troubleshooting](troubleshooting.md#build).
