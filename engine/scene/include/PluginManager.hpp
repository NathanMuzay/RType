#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "DataNode.hpp"
#include "DynamicLibrary.hpp"
#include "LoadedPlugin.hpp"
#include "PluginRegistrar.hpp"
#include "SystemFactory.hpp"

namespace engine::scene {

class PluginManager
{
public:
    static constexpr const char *REGISTER_SYMBOL = "engine_plugin_register";
    static constexpr const char *VERSION_SYMBOL = "engine_plugin_api_version";
    static constexpr const char *STAGING_FOLDER = "engine-plugins";
    static constexpr std::chrono::milliseconds CHECK_INTERVAL{500};

    using RegisterFunction = void (*)(PluginRegistrar &);
    using VersionFunction = int (*)();

    PluginManager(SystemFactory &factory, std::filesystem::path directory);
    ~PluginManager();
    PluginManager(const PluginManager &) = delete;
    PluginManager &operator=(const PluginManager &) = delete;

    void load(const DataNode &names);
    bool changed();
    void reload();
    std::vector<std::string> describe() const;
    bool empty() const;

private:
    using Clock = std::chrono::steady_clock;

    void open(LoadedPlugin &plugin);
    void close(LoadedPlugin &plugin);
    std::filesystem::path stage(const LoadedPlugin &plugin);
    static std::filesystem::file_time_type writeTime(const std::filesystem::path &path);

    SystemFactory &m_factory;
    std::filesystem::path m_directory;
    std::filesystem::path m_staging;
    std::vector<std::unique_ptr<LoadedPlugin>> m_plugins;
    std::uint64_t m_generation;
    Clock::time_point m_lastCheck;
};

} // namespace engine::scene
