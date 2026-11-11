# Replace a library or port to another platform

Each third-party library is used by exactly one module, behind engine classes. Replacing one touches that module
only.

| Library | Module | Engine classes to reimplement | Used through |
|---|---|---|---|
| SFML Graphics | `render` | `Window`, `SfmlRenderer`, `TextureManager`, `FontManager` | `IRenderer` |
| SFML Audio | `audio` | `SfmlAudio` | `IAudio` |
| SFML Window | `input` | `KeyBindings`, `InputManager` | `Action`, `UiInput` |
| Asio | `net` | `UdpTransport::Impl` (`net/internal`) | `UdpTransport` |
| nlohmann-json | `scene` | `DataNode` internals (`scene/internal`) | `DataNode` |
| pybind11 / Python | `scripting` | `ScriptEngineImpl`, `ScriptSystemImpl`, `ScriptApiBindings` | `ScriptEngine`, `ScriptSystem` |

## Example: another graphics library

1. Write a class implementing `IRenderer` (draw sprite, rectangle, text, clear, display, offset, text scale).
2. Write the window and texture loading with the new library.
3. Create it in `GameView` instead of `SfmlRenderer`.
4. Change `target_link_libraries(engine_render ...)` and `vcpkg.json`.

No system changes: they only call `IRenderer`.

## Example: another scripting language

The scripts only see the API listed in [Scripting reference](../../reference/scripting-api.md). A new language needs:

1. an engine that loads a file and calls the hooks (`on_spawn`, `on_update`, `on_damage`, `on_death`,
   `on_contact`, `run`);
2. bindings exposing `ScriptWorld` and `ScriptEntity`, which are plain C++ and stay unchanged;
3. rewriting the game scripts.

## Porting to another platform

| Part | Platform-specific code |
|---|---|
| Window, input, audio | SFML (supports Windows, Linux, macOS) |
| Network | Asio (portable) |
| Process memory in the debug overlay | `ProcessMemory` (`core/`): Windows (`psapi`) and Linux (`/proc`) |
| Python runtime | Copied by `engine_python_runtime`: the standard library path differs per OS in `engine/CMakeLists.txt` |
| Build | CMake + vcpkg triplets |

macOS is not tested: the expected work is the `ProcessMemory` implementation and the Python runtime path.
