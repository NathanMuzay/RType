#include "InputManager.hpp"

#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <cstddef>
#include <utility>

namespace engine::input {

InputManager::InputManager(KeyBindings bindings) : m_bindings(std::move(bindings))
{
}

void InputManager::update(bool windowFocused)
{
    for (std::size_t i = 0; i < KeyBindings::ACTION_COUNT; ++i) {
        Action action = static_cast<Action>(i);
        const bool down =
            windowFocused && (sf::Keyboard::isKeyPressed(m_bindings.keyFor(action)) || gamepadDown(action));
        if (m_toggle[i] && down && !m_wasDown[i])
            m_latched[i] = !m_latched[i];
        m_pressed[i] = m_toggle[i] ? m_latched[i] : down;
        m_wasDown[i] = down;
    }
}

bool InputManager::isPressed(Action action) const
{
    return m_pressed[static_cast<std::size_t>(action)];
}

KeyBindings &InputManager::bindings()
{
    return m_bindings;
}

void InputManager::setToggle(Action action, bool toggle)
{
    const std::size_t index = static_cast<std::size_t>(action);
    m_toggle[index] = toggle;
    m_latched[index] = false;
}

bool InputManager::gamepadDown(Action action) const
{
    if (!sf::Joystick::isConnected(GAMEPAD))
        return false;
    const float deadzone = m_bindings.deadzone();
    const float stickX = sf::Joystick::getAxisPosition(GAMEPAD, sf::Joystick::Axis::X);
    const float stickY = sf::Joystick::getAxisPosition(GAMEPAD, sf::Joystick::Axis::Y);
    const float padX = sf::Joystick::getAxisPosition(GAMEPAD, sf::Joystick::Axis::PovX);
    const float padY = sf::Joystick::getAxisPosition(GAMEPAD, sf::Joystick::Axis::PovY);
    switch (action) {
    case Action::Up:
        return stickY < -deadzone || padY > deadzone;
    case Action::Down:
        return stickY > deadzone || padY < -deadzone;
    case Action::Left:
        return stickX < -deadzone || padX < -deadzone;
    case Action::Right:
        return stickX > deadzone || padX > deadzone;
    default:
        break;
    }
    const auto button = m_bindings.buttonFor(action);
    return button && sf::Joystick::isButtonPressed(GAMEPAD, *button);
}

} // namespace engine::input
