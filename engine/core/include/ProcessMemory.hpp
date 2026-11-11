#pragma once

#include <cstddef>

namespace engine {

class ProcessMemory
{
public:
    static std::size_t residentBytes();
};

} // namespace engine
