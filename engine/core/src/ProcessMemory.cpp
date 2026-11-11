#include "ProcessMemory.hpp"

#ifdef _WIN32
#include <windows.h>

#include <psapi.h>
#else
#include <fstream>
#include <unistd.h>
#endif

namespace engine {

std::size_t ProcessMemory::residentBytes()
{
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS counters;
    if (!GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)))
        return 0;
    return static_cast<std::size_t>(counters.WorkingSetSize);
#else
    std::ifstream statm("/proc/self/statm");
    std::size_t total = 0;
    std::size_t resident = 0;
    if (!(statm >> total >> resident))
        return 0;
    return resident * static_cast<std::size_t>(sysconf(_SC_PAGESIZE));
#endif
}

} // namespace engine
