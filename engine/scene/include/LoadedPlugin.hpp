#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "DynamicLibrary.hpp"

namespace engine::scene {

class LoadedPlugin
{
public:
    LoadedPlugin(std::string name, std::filesystem::path source);

    const std::string &name() const;
    const std::filesystem::path &source() const;
    const std::vector<std::string> &systems() const;
    std::optional<std::filesystem::file_time_type> loadedTime() const;
    std::optional<std::filesystem::file_time_type> pendingTime() const;
    bool loaded() const;

    void attach(std::unique_ptr<DynamicLibrary> library, std::vector<std::string> systems,
        std::filesystem::file_time_type time);
    std::unique_ptr<DynamicLibrary> detach();
    void setPendingTime(std::optional<std::filesystem::file_time_type> time);

private:
    std::string m_name;
    std::filesystem::path m_source;
    std::unique_ptr<DynamicLibrary> m_library;
    std::vector<std::string> m_systems;
    std::optional<std::filesystem::file_time_type> m_loadedTime;
    std::optional<std::filesystem::file_time_type> m_pendingTime;
};

} // namespace engine::scene
