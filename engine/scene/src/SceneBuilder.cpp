#include "SceneBuilder.hpp"

#include <string>

#include "SceneError.hpp"
#include "Transform.hpp"

namespace engine::scene {

SceneBuilder::SceneBuilder(
    ecs::Registry &registry, PrefabLibrary &prefabs, const ComponentFactory &components, Vector2 area)
    : m_registry(registry), m_prefabs(prefabs), m_components(components), m_area(area), m_random(std::random_device{}())
{
}

void SceneBuilder::build(const DataNode &entries)
{
    for (const DataNode &entry : entries.items()) {
        const int count = entry.getInt("count", 1);
        if (count < 0)
            throw SceneError("SceneBuilder: " + entry.path() + ".count must not be negative");
        for (int i = 0; i < count; ++i)
            spawnEntry(entry);
    }
}

void SceneBuilder::spawnEntry(const DataNode &entry)
{
    const ecs::Entity entity = entry.has("prefab") ? m_prefabs.spawn(entry.getString("prefab")) : m_registry.spawn();
    if (entry.has("components")) {
        const DataNode components = entry.child("components");
        for (const std::string &name : components.keys())
            m_components.apply(m_registry, entity, name, components.child(name));
    }
    if (entry.getBool("scatter", false))
        scatter(entity);
}

void SceneBuilder::scatter(const ecs::Entity &entity)
{
    const float x = std::uniform_real_distribution<float>(0.f, m_area.x())(m_random);
    const float y = std::uniform_real_distribution<float>(0.f, m_area.y())(m_random);
    m_registry.emplace<ecs::Transform>(entity, x, y);
}

} // namespace engine::scene
