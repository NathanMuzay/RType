#pragma once

#include <filesystem>

namespace engine {

class AssetPath
{
public:
    static std::filesystem::path findAssets(const char *programPath);
    static std::filesystem::path findData(const char *programPath);

private:
    static std::filesystem::path findDirectory(
        const char *programPath, const std::filesystem::path &besideName, const std::filesystem::path &sourcePath);
};

} // namespace engine
