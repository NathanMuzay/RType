#include "DynamicLibrary.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "PluginError.hpp"

namespace engine {

DynamicLibrary::DynamicLibrary(const std::filesystem::path &path) : m_path(path), m_handle(nullptr)
{
#ifdef _WIN32
    m_handle = reinterpret_cast<void *>(LoadLibraryW(path.wstring().c_str()));
#else
    m_handle = dlopen(path.string().c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
    if (!m_handle)
        throw PluginError("DynamicLibrary: cannot load " + path.string() + ": " + lastError());
}

DynamicLibrary::~DynamicLibrary()
{
#ifdef _WIN32
    FreeLibrary(reinterpret_cast<HMODULE>(m_handle));
#else
    dlclose(m_handle);
#endif
}

void *DynamicLibrary::symbol(const std::string &name) const
{
#ifdef _WIN32
    void *found = reinterpret_cast<void *>(GetProcAddress(reinterpret_cast<HMODULE>(m_handle), name.c_str()));
#else
    void *found = dlsym(m_handle, name.c_str());
#endif
    if (!found)
        throw PluginError("DynamicLibrary: " + m_path.string() + " has no symbol " + name);
    return found;
}

const std::filesystem::path &DynamicLibrary::path() const
{
    return m_path;
}

std::string DynamicLibrary::fileName(const std::string &name)
{
#ifdef _WIN32
    return name + ".dll";
#else
    return name + ".so";
#endif
}

std::string DynamicLibrary::lastError()
{
#ifdef _WIN32
    return "Windows error " + std::to_string(GetLastError());
#else
    const char *message = dlerror();
    return message ? message : "unknown error";
#endif
}

} // namespace engine
