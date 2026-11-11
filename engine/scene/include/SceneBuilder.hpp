#pragma once

#include <random>

#include "ComponentFactory.hpp"
#include "DataNode.hpp"
#include "Entity.hpp"
#include "PrefabLibrary.hpp"
#include "Registry.hpp"
#include "Vector2.hpp"

namespace engine::scene {

class SceneBuilder
{
public:
    SceneBuilder(ecs::Registry &registry, PrefabLibrary &prefabs, const ComponentFactory &components, Vector2 area);

    void build(const DataNode &entries);

private:
    void spawnEntry(const DataNode &entry);
    void scatter(const ecs::Entity &entity);

    ecs::Registry &m_registry;
    PrefabLibrary &m_prefabs;
    const ComponentFactory &m_components;
    Vector2 m_area;
    std::mt19937 m_random;
};

} // namespace engine::scene
