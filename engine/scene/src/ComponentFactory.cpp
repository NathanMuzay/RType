#include "ComponentFactory.hpp"

#include <utility>

#include "SceneError.hpp"

namespace engine::scene {

void ComponentFactory::add(const std::string &name, Loader loader)
{
    if (!loader)
        throw SceneError("ComponentFactory: empty loader for " + name);
    m_loaders.insert_or_assign(name, std::move(loader));
}

bool ComponentFactory::has(const std::string &name) const
{
    return m_loaders.contains(name);
}

void ComponentFactory::apply(
    ecs::Registry &registry, const ecs::Entity &entity, const std::string &name, const DataNode &data) const
{
    const auto found = m_loaders.find(name);
    if (found == m_loaders.end())
        throw SceneError("ComponentFactory: unknown component " + name + " in " + data.path());
    found->second(registry, entity, data);
}

} // namespace engine::scene
