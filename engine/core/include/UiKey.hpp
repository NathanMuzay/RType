#pragma once

#include <cstddef>
#include <cstdint>

namespace engine {

enum class UiKey : std::uint8_t
{
    Backspace,
    Enter,
    Escape,
    Tab,
    Up,
    Down,
    Debug,
    Console,
    Left,
    Right
};

} // namespace engine
