#include "PluginManager.hpp"

#include <iostream>
#include <random>
#include <system_error>
#include <utility>

#include "PluginError.hpp"

namespace engine::scene {

PluginManager::PluginManager(SystemFactory &factory, std::filesystem::path directory)
    : m_factory(factory), m_directory(std::move(directory)),
      m_staging(std::filesystem::temp_directory_path() / STAGING_FOLDER / std::to_string(std::random_device{}())),
      m_plugins(), m_generation(0), m_lastCheck(Clock::now())
{
}

PluginManager::~PluginManager()
{
    for (const auto &plugin : m_plugins)
        close(*plugin);
    std::error_code error;
    std::filesystem::remove_all(m_staging, error);
}

void PluginManager::load(const DataNode &names)
{
    for (const DataNode &entry : names.items()) {
        const std::string name = entry.asString();
        auto plugin = std::make_unique<LoadedPlugin>(name, m_directory / DynamicLibrary::fileName(name));
        if (!std::filesystem::exists(plugin->source()))
            throw PluginError("PluginManager: plugin " + name + " not found at " + plugin->source().string());
        open(*plugin);
        m_plugins.push_back(std::move(plugin));
    }
}

bool PluginManager::changed()
{
    const Clock::time_point now = Clock::now();
    if (m_plugins.empty() || now - m_lastCheck < CHECK_INTERVAL)
        return false;
    m_lastCheck = now;
    bool ready = false;
    for (const auto &plugin : m_plugins) {
        std::error_code error;
        const auto time = std::filesystem::last_write_time(plugin->source(), error);
        if (error || (plugin->loadedTime() && time == *plugin->loadedTime())) {
            plugin->setPendingTime(std::nullopt);
            continue;
        }
        if (plugin->pendingTime() && *plugin->pendingTime() == time)
            ready = true;
        else
            plugin->setPendingTime(time);
    }
    return ready;
}

void PluginManager::reload()
{
    for (const auto &plugin : m_plugins) {
        if (plugin->loadedTime() && writeTime(plugin->source()) == *plugin->loadedTime())
            continue;
        open(*plugin);
        std::cout << "[plugins] reloaded " << plugin->name() << std::endl;
    }
}

std::vector<std::string> PluginManager::describe() const
{
    std::vector<std::string> lines;
    for (const auto &plugin : m_plugins) {
        std::string line = plugin->name() + ":";
        for (const std::string &system : plugin->systems())
            line += " " + system;
        lines.push_back(line);
    }
    return lines;
}

bool PluginManager::empty() const
{
    return m_plugins.empty();
}

void PluginManager::open(LoadedPlugin &plugin)
{
    const std::filesystem::file_time_type time = writeTime(plugin.source());
    auto library = std::make_unique<DynamicLibrary>(stage(plugin));
    const auto version = reinterpret_cast<VersionFunction>(library->symbol(VERSION_SYMBOL));
    if (version() != PluginRegistrar::API_VERSION)
        throw PluginError("PluginManager: plugin " + plugin.name() + " was built for another engine version");
    const auto registerPlugin = reinterpret_cast<RegisterFunction>(library->symbol(REGISTER_SYMBOL));
    close(plugin);
    PluginRegistrar registrar(m_factory, plugin.name());
    try {
        registerPlugin(registrar);
    } catch (...) {
        for (const std::string &system : registrar.systems())
            m_factory.remove(system);
        throw;
    }
    plugin.attach(std::move(library), registrar.systems(), time);
}

void PluginManager::close(LoadedPlugin &plugin)
{
    for (const std::string &system : plugin.systems())
        m_factory.remove(system);
    std::unique_ptr<DynamicLibrary> library = plugin.detach();
    if (!library)
        return;
    const std::filesystem::path copy = library->path();
    library.reset();
    std::error_code error;
    std::filesystem::remove(copy, error);
}

std::filesystem::path PluginManager::stage(const LoadedPlugin &plugin)
{
    std::filesystem::create_directories(m_staging);
    const std::filesystem::path copy =
        m_staging / DynamicLibrary::fileName(plugin.name() + "-" + std::to_string(++m_generation));
    std::filesystem::copy_file(plugin.source(), copy, std::filesystem::copy_options::overwrite_existing);
    return copy;
}

std::filesystem::file_time_type PluginManager::writeTime(const std::filesystem::path &path)
{
    std::error_code error;
    const auto time = std::filesystem::last_write_time(path, error);
    if (error)
        throw PluginError("PluginManager: cannot read " + path.string() + ": " + error.message());
    return time;
}

} // namespace engine::scene
