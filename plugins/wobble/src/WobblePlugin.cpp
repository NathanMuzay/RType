#include "PluginRegistrar.hpp"
#include "WobbleSystem.hpp"

ENGINE_PLUGIN_EXPORT int engine_plugin_api_version()
{
    return engine::scene::PluginRegistrar::API_VERSION;
}

ENGINE_PLUGIN_EXPORT void engine_plugin_register(engine::scene::PluginRegistrar &registrar)
{
    registrar.addSystem("Wobble", &plugins::wobble::WobbleSystem::build);
}
