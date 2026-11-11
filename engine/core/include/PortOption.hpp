#pragma once

#include <cstdint>
#include <string_view>

namespace engine {

class PortOption
{
public:
    static constexpr unsigned MAX_PORT = 65535;

    static std::uint16_t parse(std::string_view text);
};

} // namespace engine
