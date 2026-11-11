#include "SystemFactory.hpp"

#include <utility>

#include "SceneError.hpp"

namespace engine::scene {

void SystemFactory::add(const std::string &name, Builder builder)
{
    if (!builder)
        throw SceneError("SystemFactory: empty builder for " + name);
    m_builders.insert_or_assign(name, std::move(builder));
}

void SystemFactory::remove(const std::string &name)
{
    m_builders.erase(name);
}

bool SystemFactory::has(const std::string &name) const
{
    return m_builders.contains(name);
}

std::unique_ptr<ecs::ISystem> SystemFactory::create(SystemContext &context, const DataNode &entry) const
{
    const std::string name = entry.isString() ? entry.asString() : entry.getString("system");
    const auto found = m_builders.find(name);
    if (found == m_builders.end())
        throw SceneError("SystemFactory: unknown system " + name + " in " + entry.path());
    return found->second(context, entry);
}

void SystemFactory::build(SystemContext &context, const DataNode &list, ecs::SystemManager &systems) const
{
    for (const DataNode &entry : list.items())
        systems.add(create(context, entry));
}

} // namespace engine::scene
