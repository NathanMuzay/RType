# Performance

## Budgets

| Item | Limit | Where it is set |
|---|---|---|
| Simulation | 60 ticks per second, 16.6 ms per tick for all rooms | `tickRate` |
| Snapshot | 1200 bytes, checked at start | `maxPlayers`, `maxEntities`, value counts |
| Visible entities | 130 per snapshot (R-Type) | `network.maxEntities` |
| Component types | 64 | `ComponentMask` (64-bit) |
| Collision layers | 32 | `LayerTable` |

`world.can_spawn()` and `e.fire()` respect the entity limit, so a busy wave never builds an oversized snapshot.

## Snapshot size

Header 10 bytes, then 5 (tick and state) + (1 + 2 per world value) + (1 + players x (10 + 2 per player value))
+ (1 + 8 per entity). For R-Type (6 world values, 4 players with 3 values each, 130 entities):
10 + 5 + 13 + 65 + 1041 = 1134 bytes.

To show more entities: reduce `maxPlayers` or the number of values, or keep effects (particles, explosions,
stars) on the client side, where they cost nothing on the network.

## Keep work off the network

| Effect | Where |
|---|---|
| Explosions, particles, parallax, animations | Client prefabs, never `Replicated` |
| Hit flash, rotation | The `visual` byte of the entity, no extra message |
| Sounds | `SoundCue` on client prefabs, `PLAY_SOUND` only for server events |

## Scripts

Python is slower than C++. Keep `on_update` short:

- Store per-entity values on the handle (`e.speed = e.number("speed", 250.0)` in `on_spawn`) instead of reading
  params every tick.
- Use `e.every(name, seconds)` instead of doing work every tick.
- Movement that does not change can be set once with `set_velocity`: the `Movement` system then does the work
  in C++.
- A behaviour needed by many entities of many games is a candidate for a C++ system.

## Collisions

`Collision` tests every pair of entities that have a `Collider` (O(n²)), then keeps the pairs whose layers can
hit each other. With a few hundred colliders this costs well under a millisecond. Decor and effects should have
no `Collider` at all: they then cost nothing. For thousands of colliders, a grid broad phase would be the next
step ([Algorithms](../../comparative-study/algorithms-and-data-structures.md)).

## Measuring

The F3 overlay shows FPS, entity count, memory and bandwidth. On Linux, `perf top -p $(pidof r-type_server)`
shows where the server spends its time.
