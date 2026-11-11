#include "PortOption.hpp"

#include <charconv>

#include "CoreError.hpp"

namespace engine {

std::uint16_t PortOption::parse(std::string_view text)
{
    unsigned value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size() || value == 0 || value > MAX_PORT)
        throw CoreError("PortOption: port must be an integer between 1 and 65535");
    return static_cast<std::uint16_t>(value);
}

} // namespace engine
