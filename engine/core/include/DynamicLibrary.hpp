#pragma once

#include <filesystem>
#include <string>

namespace engine {

class DynamicLibrary
{
public:
    explicit DynamicLibrary(const std::filesystem::path &path);
    ~DynamicLibrary();
    DynamicLibrary(const DynamicLibrary &) = delete;
    DynamicLibrary &operator=(const DynamicLibrary &) = delete;

    void *symbol(const std::string &name) const;
    const std::filesystem::path &path() const;

    static std::string fileName(const std::string &name);

private:
    static std::string lastError();

    std::filesystem::path m_path;
    void *m_handle;
};

} // namespace engine
