# Add a system

A system applies one behaviour to every entity that has a given set of components. This guide follows the real
`GravitySystem`.

## 1. Choose the category

Systems live in `engine/ecs/systems/<category>/include/` and `.../src/`, with the categories `events`,
`physics`, `gameplay`, `presentation` and `interface`.

## 2. Write the class

`engine/ecs/systems/physics/include/GravitySystem.hpp`:

```cpp
#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class GravitySystem : public ISystem
{
public:
    explicit GravitySystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

} // namespace engine::ecs
```

`engine/ecs/systems/physics/src/GravitySystem.cpp`:

```cpp
#include "GravitySystem.hpp"

#include "Gravity.hpp"
#include "Velocity.hpp"

namespace engine::ecs {

GravitySystem::GravitySystem(Registry &registry) : m_registry(registry)
{
}

void GravitySystem::update(float dt)
{
    const auto &gravities = m_registry.getComponents<Gravity>();
    auto &velocities = m_registry.getComponents<Velocity>();
    for (std::size_t i : m_registry.view<Gravity, Velocity>()) {
        Velocity &velocity = velocities.at(i);
        velocity.setY(velocity.y() + gravities.at(i).force() * dt);
    }
}

} // namespace engine::ecs
```

Rules inside `update`:

- Multiply every change over time by `dt`.
- `view<...>()` returns a copy of the indices, so spawning entities inside the loop is safe.
- Do not `kill` entities other systems may still read: add `Destroyed` instead.
- Use `entityAt(i)` when a function needs an `Entity` handle.

## 3. Give it a builder

Systems are created by name from JSON. In `engine/scene/src/EngineSystemBuilders.cpp`, include the header and
map the name:

```cpp
factory.add("Gravity", &EngineSystemBuilders::buildSimple<ecs::GravitySystem>);
```

Ready-made builders:

| Builder | Constructor it calls |
|---|---|
| `buildSimple<S>` | `S(Registry &)` |
| `buildSpawning<S>` | `S(Registry &, IEntityFactory &, canSpawn)`: for systems that create entities |
| `buildDeathListener<S>` | `S(Registry &, const EventQueue<DeathEvent> &)` |
| `buildStoreReader<S>` | `S(Registry &, const UiStore &)` |
| `buildUiActions<S>` | `S(Registry &, const UiInput &, EventQueue<UiActionEvent> &)`: UI systems that react to input |

A system with options (like `Shake`) gets its own builder, which reads the options from the `DataNode` it
receives: `entry.getFloat("strength", 1.f)`. Everything a system may need (registry, events, prefabs, renderer,
audio, UI store, world size) is available from the `SystemContext`.

## 4. Add it to the build

Add the `.cpp` to `engine_ecs` in `engine/CMakeLists.txt`.

## 5. Use it in a game

Add its name at the right place of `serverSystems` (simulation) or of a scene's `systems` (client):

```json
"serverSystems": ["PlayerControl", "Gravity", "Movement", "Collision", "Damage", "Script"]
```

Then add a row to the [System reference](../../reference/systems.md).
