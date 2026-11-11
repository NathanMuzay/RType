# Coding conventions

These rules keep the code consistent, whoever wrote it. Every pull request must follow them; reviewers refuse
code that does not.

## Language

- Code, commit messages and documentation are written in **English**.
- **No comments in the code**, Doxygen included. Names must explain the code; explanations go in this
  documentation.
- C++20, no compiler extensions.

## Naming

| Element | Style | Example |
|---|---|---|
| Classes and enums | `PascalCase` | `GameServer`, `MessageType` |
| Functions and methods | `camelCase` | `takeEvents()`, `onGround()` |
| Variables and parameters | `camelCase` | `entityCount` |
| Private members | `m_` + `camelCase` | `m_registry` |
| Constants | `UPPER_SNAKE_CASE`, as `static constexpr` class members | `MAX_PACKET_BYTES` |
| `enum class` values | `PascalCase` | `MessageType::Connect` |
| Namespaces | `lowercase` | `engine::network` |
| Python functions and variables | `snake_case` | `start_moving`, `boss_health` |
| Python constants | `UPPER_SNAKE_CASE` | `FLAP_SPEED` |
| JSON keys | `camelCase` | `restartDelay`, `maxEntities` |

## Classes and types

- **Classes only, no `struct`.** Data holders are classes with private members and accessors.
- **No lambdas.** Use named member functions, with `std::bind_front` when a callable is needed.
- **No global variables and no namespace-level constants**, `inline` ones included. Constants are
  `static constexpr` members of the class that uses them.
- **No anonymous namespaces.** Helpers are private static member functions.
- `enum class` only.
- No `new` / `delete`: RAII, `std::unique_ptr`, containers.

## Files

- One main class per file, named after the class: `GameServer.hpp`, `GameServer.cpp`.
- A long class may be split in several `.cpp` files named after their topic: `GameServer.cpp`,
  `GameServerRooms.cpp`.
- **Headers contain no function body.** Templates go in a `.tpp` file with the same name, included at the end of
  the header.
- Headers in `include/`, sources in `src/`, private headers of a module in `internal/`.
- Every header starts with `#pragma once`.
- Includes use the file name only: `#include "GameServer.hpp"`. **File names are unique in the whole engine.**
- Every new `.cpp` is added to `engine/CMakeLists.txt` in the same pull request.

## Errors

- Errors are exceptions deriving from `engine::EngineError`, one class per module (`SceneError`,
  `NetError`...). Never throw `std::runtime_error` or other standard exceptions directly.
- Messages start with the class that raises them: `"Registry: component type not registered"`.
- Launchers catch everything and return **84**.
- A thread function catches all its exceptions and forwards them to the main thread.

## C++ rules

- `nullptr`, `const` references for objects passed as parameters, `override` on virtual functions.
- Fixed-size integers (`std::uint16_t`...) for anything sent on the network, written field by field, never by
  copying memory.
- Time-based changes use `dt` (`position += velocity * dt`), never the frame rate.
- Third-party types never appear in a public header outside the module that wraps them.

## Formatting

Formatting is automatic: the rules are in `.clang-format` at the repository root (clang-format 18). Format before
committing:

```sh
pip install clang-format==18.1.8
find engine -name '*.cpp' -o -name '*.hpp' -o -name '*.tpp' | xargs clang-format --style=file -i
```

Most IDEs (VS Code, CLion, Visual Studio) apply the same file on save. The CI refuses a pull request whose code
is not formatted. The main rules:

- 4 spaces, no tabs, lines up to 120 characters.
- Opening brace on its own line for functions and classes, on the same line for control statements:

```cpp
void GameServer::poll()
{
    for (const net::Datagram &datagram : m_transport.poll()) {
        receive(datagram, now);
    }
}
```

## Data and scripts

- One prefab per entity type, named in `snake_case` (`bydo_sine`, `boss_vessel`).
- Python scripts start with a docstring that explains the behaviour and its params; module-level values are
  constants only.
- New assets come with their license file ([Assets](../guides/content/assets.md)).

## Commits

Conventional commits, in English, imperative:

```text
feat(network): add console messages
fix(client): keep the pause menu above the HUD
docs: rewrite the protocol RFC for version 7
refactor(ecs): split GameplaySystems into one file per system
build: add the solo launcher
```

| Prefix | For |
|---|---|
| `feat` | A new feature |
| `fix` | A bug fix |
| `refactor` | Code change without behaviour change |
| `docs` | Documentation only |
| `build` | CMake, vcpkg, scripts |
| `chore` | Anything else (cleanup, backups) |

The scope is the module or folder: `ecs`, `network`, `client`, `scripting`, `game`, `skyhop`.

See [Git workflow](git-workflow.md) for branches and reviews.
