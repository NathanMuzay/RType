# Onboarding: joining the team

This page is for someone who joins the project, at any time. Follow it in order: at the end of the first day you
can build, play, read the code and make a first change.

## The project in six sentences

1. The repository contains an **engine** written in C++ (`engine/`) and **games** written in data (`game/`,
   `game2/`).
2. A game is JSON files (entities, scenes, HUD) plus Python scripts (behaviours). It contains no C++.
3. R-Type is played online: `r-type_server` simulates, `r-type_client` displays and sends the buttons.
4. Skyhop is played solo: `skyhop` runs the same simulation inside the client.
5. Game objects are **entities** made of **components** and updated by **systems** (ECS).
6. Client and server speak a binary UDP protocol documented as an RFC.

## Day 1

### Access

Ask a team member for:

| What | Why |
|---|---|
| Write access to the GitHub repository | Push branches, open pull requests |
| The team's communication channel | Questions, reviews, planning |
| The list of open issues and who works on what | Pick a first task without overlapping |

### Set up and build

1. [Installation](../getting-started/installation.md): compiler, CMake, vcpkg.
2. [Building](../getting-started/building.md): the first build downloads the dependencies (several minutes).
3. [Running](../getting-started/running.md): start a server and two clients, play a stage; start Skyhop.

If anything fails, check [Troubleshooting](../getting-started/troubleshooting.md) first.

### Read

| Order | Page | Time |
|---|---|---|
| 1 | [Architecture overview](../architecture/overview.md) | 15 min |
| 2 | [Data-driven games](../architecture/data-driven.md) | 10 min |
| 3 | [ECS](../architecture/ecs.md) | 15 min |
| 4 | [Coding conventions](conventions.md) | 10 min |
| 5 | [Git workflow](git-workflow.md) | 5 min |
| 6 | [Code map](../reference/code-map.md) | 10 min |

Then open these files to see the real code:

| File | What you learn |
|---|---|
| `CMakeLists.txt` (root) | How games are declared |
| `engine/apps/src/ServerMain.cpp` | How a program starts the engine |
| `engine/simulation/src/Simulation.cpp` | What happens in one server tick |
| `engine/ecs/systems/physics/src/MovementSystem.cpp` | The shape of every system |
| `game/data/rtype_server_prefabs.json` | What entities are made of |
| `game/scripts/campaign.py` | How a game is driven by a script |

## First change

Pick one of these depending on your role, it touches every layer without risk:

| Role | First task |
|---|---|
| Content (JSON, Python) | [Add an enemy](../guides/content/add-an-enemy.md) and put it in a wave of stage 1 |
| Engine (C++) | Add a client-only component and its system, for example a blinking text ([Add a component](../guides/engine/add-a-component.md)) |
| Network | Read the [RFC](../reference/protocol.md) and decode a captured datagram by hand with Wireshark |

Open a draft pull request early: reviews are the fastest way to learn the conventions.

## Who owns what

| Area | Folder | Track |
|---|---|---|
| Engine core, ECS, scenes, simulation, gameplay systems | `engine/core`, `engine/ecs`, `engine/scene`, `engine/simulation` | Architecture and gameplay |
| Network, protocol, server, lobby | `engine/net`, `engine/network` | Network |
| Game content (levels, enemies, bosses, HUD) | `game/`, `game2/` | Content |
| Documentation | `docs/` | Everyone |

The member responsible for each track is the assignee of its issues on the project board.

Ask in the team channel when in doubt: a five-minute question avoids a day of rework.

## Rules that are easy to miss

- No comments in the code, no function body in a header, no struct, no lambda (see [conventions](conventions.md)).
- Every new `.cpp` goes in `engine/CMakeLists.txt`.
- The engine never contains anything specific to one game.
- The client never decides a gameplay outcome.
- Never push directly to `main`.
