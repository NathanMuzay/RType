# Add a component

Add a component to the engine only when a feature is **generic** (useful to several games) and cannot be done
with existing components plus a script. A component is plain data; its behaviour goes in a system
([Add a system](add-a-system.md)).

This guide follows the real `Gravity` component.

## 1. Choose the category

| Category | For |
|---|---|
| `physics` | Position, movement, collisions |
| `combat` | Health, damage, weapons |
| `gameplay` | Players, rules, match, scripts, maps |
| `presentation` | Drawing, animation, sound |
| `interface` | Menus and HUD |

Files go in `engine/ecs/components/<category>/include/` and `.../src/`. The file name must be unique in the
whole engine.

## 2. Write the class

`engine/ecs/components/physics/include/Gravity.hpp`:

```cpp
#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class Gravity : public IComponent
{
public:
    explicit Gravity(float force);

    float force() const;

private:
    float m_force;
};

} // namespace engine::ecs
```

`engine/ecs/components/physics/src/Gravity.cpp`:

```cpp
#include "Gravity.hpp"

namespace engine::ecs {

Gravity::Gravity(float force) : m_force(force)
{
}

float Gravity::force() const
{
    return m_force;
}

} // namespace engine::ecs
```

Rules: a class (no struct), derives from `IComponent`, members prefixed with `m_`, no function body in the
header, no comment in the code (see [Coding conventions](../../contributing/conventions.md)).

## 3. Register the type

In `engine/ecs/components/src/ComponentRegistration.cpp`, include the header and add one line to
`registerEngineComponents`:

```cpp
registry.registerComponent<Gravity>();
```

## 4. Make it loadable from JSON

1. Declare a loader in `engine/scene/include/EngineComponentLoaders.hpp`:

    ```cpp
    static void loadGravity(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    ```

2. Implement it in the file of its category (`EngineComponentLoadersGameplay.cpp` for gameplay and physics,
   `EngineComponentLoadersPresentation.cpp`, `EngineComponentLoadersInterface.cpp`):

    ```cpp
    void EngineComponentLoaders::loadGravity(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
    {
        registry.emplace<ecs::Gravity>(entity, data.getFloat("force"));
    }
    ```

    Use `getFloat("key", default)` for optional fields. A missing required field gives a clear `SceneError`.

3. Map its JSON name in the matching `register...` function of `EngineComponentLoaders.cpp`
   (`registerGameplay`, `registerRules`, `registerPresentation` or `registerInterface`):

    ```cpp
    factory.add("Gravity", &EngineComponentLoaders::loadGravity);
    ```

## 5. Add it to the build

Add the source to `engine_ecs` in `engine/CMakeLists.txt`, in its category:

```cmake
    ecs/components/physics/src/Gravity.cpp
```

## 6. Document it

Add a row to the [Component reference](../../reference/components.md) with its fields and defaults.

## Sending it to clients

Components are **not** sent over the network. Clients only receive the `EntityState` of replicated entities
(id, kind, position, visual byte) and the values of the HUD. If clients must see new data:

- for a per-entity look, use a different `kind`, or the `visual` byte;
- for a number shown in the HUD, use a world value or a player value from a script;
- only if neither fits, change the protocol: new fields, version bump, RFC update
  ([Add a network message](add-a-network-message.md)).
