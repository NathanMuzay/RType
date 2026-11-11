# Troubleshooting

## Build

| Problem | Solution |
|---|---|
| `CMAKE_TOOLCHAIN_FILE does not exist` | vcpkg is not where CMake looks. Clone it into `.vcpkg` at the repository root, or pass `-DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake`. Delete the build folder before configuring again |
| The first configuration takes very long | Normal: vcpkg builds SFML, Python and the other dependencies once. Later builds reuse them |
| `Could not find a package configuration file provided by "SFML"` | The toolchain file was not given at the first configuration. Delete the build folder and configure again with it |
| Linux: errors about X11, OpenGL, udev or Xrandr | Install the development packages ([Installation](installation.md)) |
| `No such file` for a source after pulling | Files were moved or renamed. Reconfigure: `cmake -S . -B build`, or delete the build folder |
| Linking errors after switching branches | Stale objects. Delete the build folder and rebuild |
| Windows: `MSB8020` or missing toolset | Install "Desktop development with C++" in the Visual Studio Installer |

## Start-up

| Problem | Solution |
|---|---|
| The program exits with code 84 and one line | Read the line: it names the file and the key at fault ([Debugging, load errors](../guides/engine/debugging.md#load-errors)) |
| `DataNode: cannot open .../data/...` | Data not copied next to the executable: rebuild (the build copies `data`, `scripts` and `assets`) |
| `ScriptEngine: cannot load ...` | A script is missing or has a syntax error |
| Python errors at start (`encodings`, `No module named`) | The `python/` folder next to the executable is missing: rebuild the `engine_python_runtime` target |
| Windows: "a DLL is missing" | Start the program from the `out\windows\bin\Release` folder, where the DLLs are copied |
| Windows asks which app to open the file with | Type the full name with `.exe`: `.\r-type_client.exe` |
| WSL: the window does not open | Use WSLg (Windows 11) or an X server, or play with the Windows build |

## Network

| Problem | Solution |
|---|---|
| Client stays on "CONNECTING..." | Server not started, wrong `-h` or `-p`, or firewall ([Network play](network-play.md)) |
| "SERVER UNREACHABLE OR CONNECTION LOST" right away | Client and server from different versions: rebuild both |
| "THE ROOM STOPPED ANSWERING" | The server stopped or the network dropped for more than 4 seconds |
| Works on one PC, not between two | Firewall on the server PC, or Wi-Fi with client isolation |
| WSL server not reachable from other PCs | WSL NAT network: run the Windows server, or set `networkingMode=mirrored` |

## In game

| Problem | Solution |
|---|---|
| An enemy does nothing | Its script failed: look for `[script]` on the server output |
| A sprite is missing (nothing drawn) | Wrong `frame`, or texture key not declared in `textures` |
| Keys do not respond | The window must have the focus. Check `controls` in the settings file |
| Gamepad does not respond | Plug it in before starting, check `gamepad` button numbers in the settings |
| Console commands refused | You are not the room owner, or the room was created with CHEATS OFF |
| Sound too loud or silent | `set soundVolume 50`, `set musicVolume 30` in the console, or the settings file |
| Text too small | Raise `textScale` in the settings file (for example `1.3`) |
