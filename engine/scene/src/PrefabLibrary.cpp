#include "PrefabLibrary.hpp"

#include "SceneError.hpp"

namespace engine::scene {

PrefabLibrary::PrefabLibrary(ecs::Registry &registry, const ComponentFactory &factory)
    : m_registry(registry), m_factory(factory)
{
}

void PrefabLibrary::load(const DataNode &prefabs)
{
    for (const std::string &name : prefabs.keys()) {
        const DataNode prefab = prefabs.child(name);
        for (const std::string &component : prefab.keys()) {
            if (!m_factory.has(component))
                throw SceneError("PrefabLibrary: unknown component " + component + " in " + prefab.path());
        }
        m_prefabs.insert_or_assign(name, prefab);
    }
}

bool PrefabLibrary::has(const std::string &name) const
{
    return m_prefabs.contains(name);
}

ecs::Entity PrefabLibrary::spawn(const std::string &name)
{
    const auto found = m_prefabs.find(name);
    if (found == m_prefabs.end())
        throw SceneError("PrefabLibrary: unknown prefab " + name);
    const ecs::Entity entity = m_registry.spawn();
    try {
        for (const std::string &component : found->second.keys())
            m_factory.apply(m_registry, entity, component, found->second.child(component));
    } catch (...) {
        m_registry.kill(entity);
        throw;
    }
    return entity;
}

} // namespace engine::scene
