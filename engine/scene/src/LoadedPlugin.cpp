#include "LoadedPlugin.hpp"

#include <utility>

namespace engine::scene {

LoadedPlugin::LoadedPlugin(std::string name, std::filesystem::path source)
    : m_name(std::move(name)), m_source(std::move(source)), m_library(), m_systems(), m_loadedTime(), m_pendingTime()
{
}

const std::string &LoadedPlugin::name() const
{
    return m_name;
}

const std::filesystem::path &LoadedPlugin::source() const
{
    return m_source;
}

const std::vector<std::string> &LoadedPlugin::systems() const
{
    return m_systems;
}

std::optional<std::filesystem::file_time_type> LoadedPlugin::loadedTime() const
{
    return m_loadedTime;
}

std::optional<std::filesystem::file_time_type> LoadedPlugin::pendingTime() const
{
    return m_pendingTime;
}

bool LoadedPlugin::loaded() const
{
    return m_library != nullptr;
}

void LoadedPlugin::attach(
    std::unique_ptr<DynamicLibrary> library, std::vector<std::string> systems, std::filesystem::file_time_type time)
{
    m_library = std::move(library);
    m_systems = std::move(systems);
    m_loadedTime = time;
    m_pendingTime.reset();
}

std::unique_ptr<DynamicLibrary> LoadedPlugin::detach()
{
    m_systems.clear();
    return std::move(m_library);
}

void LoadedPlugin::setPendingTime(std::optional<std::filesystem::file_time_type> time)
{
    m_pendingTime = time;
}

} // namespace engine::scene
