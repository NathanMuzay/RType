#include "AssetPath.hpp"

namespace engine {

std::filesystem::path AssetPath::findAssets(const char *programPath)
{
    return findDirectory(programPath, "assets", "assets");
}

std::filesystem::path AssetPath::findData(const char *programPath)
{
    return findDirectory(programPath, "data", std::filesystem::path("game") / "data");
}

std::filesystem::path AssetPath::findDirectory(
    const char *programPath, const std::filesystem::path &besideName, const std::filesystem::path &sourcePath)
{
    std::filesystem::path besideProgram = std::filesystem::absolute(programPath).parent_path() / besideName;
    if (std::filesystem::exists(besideProgram))
        return besideProgram;
    return std::filesystem::current_path() / sourcePath;
}

} // namespace engine
