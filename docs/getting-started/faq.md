# FAQ

## About the project

**Is the engine specific to R-Type?**
No. The engine contains no R-Type code: R-Type and Skyhop are folders of JSON, Python and assets loaded at run
time. See [Data-driven games](../architecture/data-driven.md).

**Why Python for behaviours, and not only JSON?**
JSON describes what entities are. Behaviours (timelines, patterns, conditions) need code, and Python is readable
by content creators and needs no compilation. See [Design decisions](../architecture/decisions.md).

**Why UDP and not TCP?**
A real-time game prefers fresh data to complete data. A lost snapshot is replaced by the next one 50 ms
later, and the few messages that must arrive are acknowledged and resent by the protocol itself, while TCP would delay everything behind it. See [Networking study](../comparative-study/networking.md).

**Can it be played over the Internet?**
The protocol works over any IP network, but it has no encryption and no NAT traversal. It is designed for LAN
play. A server with a public address and an open UDP port would work.

**How many players?**
4 per room in co-op, 2 in versus, up to 48 rooms and 192 connected clients per server (settings of the game
file).

## Creating content

**Do I need to recompile to change a level?**
No. Edit the JSON or Python files in `game/`, then rebuild only to copy them next to the executables (or edit
the copies in `build/bin/` directly while testing).

**How do I make a new enemy?**
A server prefab, a client prefab, a `kinds` entry and usually a script: [Add an enemy](../guides/content/add-an-enemy.md).

**How do I make a whole new game?**
[Create a game from scratch](../guides/content/create-a-game.md).

**My script error stops my enemy but not the game, why?**
On purpose: a failing script is disabled for that entity only, and the traceback is printed with `[script]`.

## Engine development

**When should something become a C++ component or system?**
When it is generic (useful to several games) or too slow in Python. Otherwise keep it in a script.

**Where do I start reading the code?**
[Onboarding](../contributing/onboarding.md) and the [Code map](../reference/code-map.md).

**Why are there no comments in the code?**
Team convention: names explain the code, and explanations live in this documentation where they are easier to
keep up to date. See [Conventions](../contributing/conventions.md).
