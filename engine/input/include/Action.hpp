#pragma once

#include <cstdint>

namespace engine::input {

enum class Action : std::uint8_t
{
    Up,
    Down,
    Left,
    Right,
    Fire,
    Special
};

} // namespace engine::input
