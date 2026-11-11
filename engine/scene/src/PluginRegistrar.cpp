#include "PluginRegistrar.hpp"

#include <utility>

#include "PluginError.hpp"

namespace engine::scene {

PluginRegistrar::PluginRegistrar(SystemFactory &factory, std::string plugin)
    : m_factory(factory), m_plugin(std::move(plugin)), m_systems()
{
}

void PluginRegistrar::addSystem(const std::string &name, SystemFactory::Builder builder)
{
    if (m_factory.has(name))
        throw PluginError("PluginRegistrar: plugin " + m_plugin + " redefines the system " + name);
    m_factory.add(name, std::move(builder));
    m_systems.push_back(name);
}

const std::string &PluginRegistrar::plugin() const
{
    return m_plugin;
}

const std::vector<std::string> &PluginRegistrar::systems() const
{
    return m_systems;
}

} // namespace engine::scene
