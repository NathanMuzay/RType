#pragma once

#include <string>
#include <vector>

#include "SystemFactory.hpp"

#ifdef _WIN32
#define ENGINE_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
#define ENGINE_PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
#endif

namespace engine::scene {

class PluginRegistrar
{
public:
    static constexpr int API_VERSION = 1;

    PluginRegistrar(SystemFactory &factory, std::string plugin);

    void addSystem(const std::string &name, SystemFactory::Builder builder);
    const std::string &plugin() const;
    const std::vector<std::string> &systems() const;

private:
    SystemFactory &m_factory;
    std::string m_plugin;
    std::vector<std::string> m_systems;
};

} // namespace engine::scene
