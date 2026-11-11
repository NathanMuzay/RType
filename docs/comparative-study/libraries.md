# Libraries and tools

## Language: C++20

| Option | Strengths | Weaknesses |
|---|---|---|
| **C++20** | Native performance, deterministic memory (RAII), direct access to every game library, `std::span`, `std::variant`, concepts and `requires` | Memory safety is the developer's responsibility |
| C | Simple, portable | No RAII, no containers, more error-prone code |
| Rust | Memory safety | Smaller game ecosystem for this scope, not the course language |
| C# / Java | Productivity | Garbage-collector pauses, heavier runtime |

**Choice:** C++20, required by the project and suited to a real-time engine.

## Graphics, window, audio and input: SFML 3

| Option | Strengths | Weaknesses |
|---|---|---|
| **SFML 3** | C++ API, 2D graphics, audio, window, keyboard and gamepad in one library, cross-platform, in vcpkg | 2D only, smaller community than SDL |
| SDL2 / SDL3 | Very widespread, excellent controller support | C API, basic rendering helpers |
| Raylib | Very simple, includes 3D | C API, global state |
| Full engines (Unity, Godot, Unreal) | Complete tooling | Forbidden by the subject |

**Choice:** SFML 3, kept behind `IRenderer`, `IAudio` and the input module: replacing it touches three modules
and no system.

## Networking: Asio (standalone)

| Option | Strengths | Weaknesses |
|---|---|---|
| **Asio** | Portable sockets on Linux and Windows, asynchronous model, header-only | Callback style takes some learning |
| BSD sockets / Winsock | No dependency | Two APIs to maintain, must be wrapped anyway |
| SFML Network | Already a client dependency | Would make the server depend on SFML |
| ENet | Reliable channels on UDP | Hides part of the protocol design required by the subject |

**Choice:** Asio, private to `engine_net`, with one I/O thread that never blocks the game loop.

## Game data: JSON with nlohmann-json

| Option | Strengths | Weaknesses |
|---|---|---|
| **JSON** | Readable, editable by hand, supported by every editor, easy to generate from tools | No comments, verbose for large maps |
| YAML | Comments, shorter | Indentation errors, slower parsers |
| XML | Schemas | Very verbose |
| Custom binary | Compact, fast | Needs an editor to change anything |
| Lua tables | Data and code in one language | A second scripting language next to Python |

| JSON library | Strengths | Weaknesses |
|---|---|---|
| **nlohmann-json** | Most used C++ JSON library, simple API, good error messages | Not the fastest (irrelevant: files are read once) |
| RapidJSON | Very fast | Verbose API |
| simdjson | Fastest | Read-only, more complex |

**Choice:** JSON read by nlohmann-json, hidden behind `DataNode`, which adds the file and path of the faulty value
to every error.

## Scripting: Python 3 with pybind11

| Option | Strengths | Weaknesses |
|---|---|---|
| **Python 3** | Known by most developers and content creators, readable, huge standard library | Slower than Lua, bigger runtime |
| Lua | Small, fast, made for embedding, standard in games | Less known by the team, 1-based arrays, small standard library |
| JavaScript (QuickJS, V8) | Widely known | Heavier to embed (V8), less common in game engines |
| Behaviours in C++ only | Fastest | Every new enemy needs a compilation; content creators need C++ |
| Visual scripting | No code | Needs an editor, long to build |

| Binding library | Strengths | Weaknesses |
|---|---|---|
| **pybind11** | Header-only, modern C++ API, automatic conversions, in vcpkg | Compile time |
| Python C API | No dependency | Very verbose, manual reference counting |
| Boost.Python | Mature | Pulls Boost |

**Choice:** Python 3 embedded with pybind11, only in `engine_scripting`. The Python standard library is copied
next to the executables, so players install nothing. Script errors are isolated per entity.

## Package manager: vcpkg

| Option | Strengths | Weaknesses |
|---|---|---|
| **vcpkg** | Manifest mode (`vcpkg.json`), pinned baseline, CMake integration, good Windows support | First build compiles dependencies |
| Conan | Powerful profiles, prebuilt binaries | More configuration |
| CMake CPM / FetchContent | No external tool | Rebuilds large libraries (SFML, Python) per project |

**Choice:** vcpkg with a pinned `builtin-baseline`: the same versions on every machine, nothing installed system
wide.

## Build system: CMake

| Option | Strengths | Weaknesses |
|---|---|---|
| **CMake** | Industry standard, generates Makefiles, Ninja and Visual Studio solutions, native vcpkg support | Verbose syntax |
| Meson | Clean syntax, fast | Less integrated with vcpkg and IDEs |
| xmake | Integrated package manager | Smaller ecosystem |

**Choice:** CMake. One library per engine module enforces dependency rules at build time, and the function
`engine_add_game` turns a folder of data into executables.

## Documentation: Markdown + MkDocs Material

| Option | Strengths | Weaknesses |
|---|---|---|
| **MkDocs Material** | Plain Markdown in the repository, search, navigation, Mermaid diagrams, static website, accessible theme | Needs Python to build the site |
| Sphinx | Very powerful | reStructuredText, heavier setup |
| Doxygen | Generated from code | API reference only, and the code has no comments by convention |
| GitBook / wiki | Collaborative editing | Separated from the code history |

**Choice:** Markdown versioned with the code, rendered by MkDocs Material. GitHub also renders the pages directly.
