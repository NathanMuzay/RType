# Algorithms and data structures

## Object model

| Option | Strengths | Weaknesses |
|---|---|---|
| Inheritance hierarchy (`GameObject` -> `Ship`, `Enemy`...) | Familiar | Deep, rigid trees; behaviours cannot be combined or changed at run time |
| **Entity Component System** | Composition, run-time changes, data described in JSON | Behaviour spread across systems |

**Choice:** ECS. It is what makes the engine data-driven: a prefab is a list of components, so a new enemy is new
JSON, not a new class. Run-time composition example: `Invulnerable` is added on respawn and removed after 2 s.

## ECS storage

| Variant | How it works | Strengths | Weaknesses |
|---|---|---|---|
| Fixed struct of arrays per game | One hand-written array per component in a game class | Simple | The engine must know the game's types: not reusable |
| **`SparseArray<T>` = `vector<optional<T>>` per type + 64-bit mask** (ours) | One storage per registered type, indexed by entity; a mask per entity for queries | O(1) access, contiguous loops, generic, queries compare integers | Storage sized up to the highest index for each type; 64 types at most |
| Sparse set (EnTT) | Dense packed arrays plus a sparse index | Fastest iteration over large sets | More complex |
| Archetypes (Unity DOTS, flecs) | Entities grouped by exact component set | Best for very large worlds | Moving entities between tables at every change |

**Choice:** sparse arrays and masks. Worlds hold a few hundred entities, where the difference with sparse sets is
not measurable, and the code stays short and readable. The registry is generic (templates on the component type),
so the engine stores components of any game without knowing them in advance. Systems only use `view`, `get`,
`emplace` and `kill`, so the storage can be replaced without changing them.

## Entity handles

| Option | Risk |
|---|---|
| Raw pointers | Dangling after destruction |
| Plain integer ids | An old id silently targets the new entity reusing the slot |
| **Generational index** (index + generation) | A stale handle is detected because the generation differs |

Freed indices are reused from a stack (last freed, first reused), which keeps recent rows hot in cache. The
generation makes this immediate reuse safe, which matters because scripts keep handles across ticks.

## Deferred destruction

Entities are tagged `Destroyed` during the tick and removed by a cleanup step at the end. Systems that react to a
death (score, lives, `on_death`, death effects) still see the entity. Killing immediately would force every system
to check for missing components.

## Data-driven factories

| Option | Effect |
|---|---|
| Hard-coded factory functions (`createBydo()`) | Every new entity type needs C++ |
| **Name-to-function maps** (`ComponentFactory`, `SystemFactory`) | JSON names select loaders and builders. New entity types and system orders are data |

## Time step

| Option | Effect |
|---|---|
| Per-frame movement | Speed depends on CPU speed |
| Variable `dt` | Correct speed, but results depend on frame timing; large steps can skip collisions |
| **Fixed step with accumulator** | Identical 1/60 s steps, catch-up bounded to 0.25 s per frame |

## Collision detection

| Shape test | Use |
|---|---|
| **Axis-aligned bounding boxes** | Four comparisons; matches rectangular sprites |
| Circles | Cheap, poor fit for long missiles |
| Separating axis theorem | Rotated polygons, not needed |

| Broad phase | Cost |
|---|---|
| **All pairs, then layer filter** (ours) | O(n²): about 20,000 box tests for 200 colliders, well under a millisecond |
| Uniform grid / quadtree | O(n) on average; worthwhile for thousands of entities |

**Push-out.** `Aabb::separation` returns the smallest move (on x or on y) that separates two boxes. `Solid` pushes
movers out with it, and `Bounce` reflects the velocity on the axis of the push. `on_ground()` uses it on a box
moved 2 pixels down: a push upward means the entity stands on a solid.

## Network data structures

- Snapshots are lists of 8-byte entity records; the entity limit keeps a snapshot under 1200 bytes, checked at
  start-up.
- Sequence numbers and ticks use modular comparison (`(a - b) mod 2^32 < 2^31`) so counters can wrap.
- The receive queue is swapped out in one operation, keeping the locked section minimal.
- Messages are alternatives of a `std::variant`, processed with visitors.

## Design patterns

| Pattern | Where |
|---|---|
| Entity Component System | `engine_ecs`, all game logic |
| Factory (by name) | `ComponentFactory`, `SystemFactory`, `PrefabLibrary`, `SimulationFactory` |
| Prototype | Prefabs: entities are copies of a JSON template |
| Interface / strategy | `ISystem`, `IRenderer`, `IAudio`, `IGameSession`, `ISessionFactory` |
| Visitor | `MessageValidator`, `MessageWriter`, `TokenReader` on `Packet` |
| Private implementation (pimpl) | `UdpTransport::Impl`, `ScriptEngineImpl`, `DataNode` internals |
| Observer (event queues) | `EventQueue<CollisionEvent>`, `EventQueue<DeathEvent>`... |
| State machine | Match (waiting, playing, over), client connection, campaign phases |
| Facade | `GameView` (window, resources, scenes), `ScriptWorld` (engine for Python) |
| RAII | Systems in `unique_ptr`, the transport thread joined in the destructor |
