#pragma once

#include <array>
#include <cstdint>

#include "Action.hpp"
#include "KeyBindings.hpp"

namespace engine::input {

class InputManager
{
public:
    explicit InputManager(KeyBindings bindings = KeyBindings());

    void update(bool windowFocused);
    bool isPressed(Action action) const;

    KeyBindings &bindings();
    void setToggle(Action action, bool toggle);

private:
    static constexpr unsigned int GAMEPAD = 0;

    bool gamepadDown(Action action) const;

    KeyBindings m_bindings;
    std::array<bool, KeyBindings::ACTION_COUNT> m_pressed{};
    std::array<bool, KeyBindings::ACTION_COUNT> m_toggle{};
    std::array<bool, KeyBindings::ACTION_COUNT> m_latched{};
    std::array<bool, KeyBindings::ACTION_COUNT> m_wasDown{};
};

} // namespace engine::input
