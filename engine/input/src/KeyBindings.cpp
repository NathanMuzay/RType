#include "KeyBindings.hpp"

#include <cctype>
#include <utility>

#include "InputError.hpp"

namespace engine::input {

KeyBindings::KeyBindings()
    : m_keys{sf::Keyboard::Scancode::Up, sf::Keyboard::Scancode::Down, sf::Keyboard::Scancode::Left,
          sf::Keyboard::Scancode::Right, sf::Keyboard::Scancode::Space, sf::Keyboard::Scancode::LShift},
      m_buttons{}, m_deadzone(DEFAULT_DEADZONE)
{
}

void KeyBindings::bind(Action action, sf::Keyboard::Scancode key)
{
    m_keys[static_cast<std::size_t>(action)] = key;
}

sf::Keyboard::Scancode KeyBindings::keyFor(Action action) const
{
    return m_keys[static_cast<std::size_t>(action)];
}

void KeyBindings::bindButton(Action action, unsigned int button)
{
    m_buttons[static_cast<std::size_t>(action)] = button;
}

std::optional<unsigned int> KeyBindings::buttonFor(Action action) const
{
    return m_buttons[static_cast<std::size_t>(action)];
}

void KeyBindings::setDeadzone(float deadzone)
{
    m_deadzone = deadzone;
}

float KeyBindings::deadzone() const
{
    return m_deadzone;
}

sf::Keyboard::Scancode KeyBindings::keyFromName(const std::string &name)
{
    if (name.size() == 1 && std::isalpha(static_cast<unsigned char>(name[0]))) {
        const int offset = std::toupper(static_cast<unsigned char>(name[0])) - 'A';
        return static_cast<sf::Keyboard::Scancode>(static_cast<int>(sf::Keyboard::Scancode::A) + offset);
    }
    if (name.size() == 1 && std::isdigit(static_cast<unsigned char>(name[0]))) {
        if (name[0] == '0')
            return sf::Keyboard::Scancode::Num0;
        return static_cast<sf::Keyboard::Scancode>(static_cast<int>(sf::Keyboard::Scancode::Num1) + name[0] - '1');
    }
    for (const auto &[keyName, key] : namedKeys()) {
        if (name == keyName)
            return key;
    }
    throw InputError("KeyBindings: unknown key name " + name);
}

std::string KeyBindings::nameOf(sf::Keyboard::Scancode key)
{
    const int code = static_cast<int>(key);
    const int letter = code - static_cast<int>(sf::Keyboard::Scancode::A);
    if (letter >= 0 && letter < LETTER_COUNT)
        return std::string(1, static_cast<char>('A' + letter));
    if (key == sf::Keyboard::Scancode::Num0)
        return "0";
    const int digit = code - static_cast<int>(sf::Keyboard::Scancode::Num1);
    if (digit >= 0 && digit < DIGIT_COUNT - 1)
        return std::string(1, static_cast<char>('1' + digit));
    for (const auto &[keyName, named] : namedKeys()) {
        if (named == key)
            return keyName;
    }
    return {};
}

const std::array<std::pair<const char *, sf::Keyboard::Scancode>, KeyBindings::NAMED_KEY_COUNT> &
KeyBindings::namedKeys()
{
    static const std::array<std::pair<const char *, sf::Keyboard::Scancode>, NAMED_KEY_COUNT> named = {{
        {"Up", sf::Keyboard::Scancode::Up},
        {"Down", sf::Keyboard::Scancode::Down},
        {"Left", sf::Keyboard::Scancode::Left},
        {"Right", sf::Keyboard::Scancode::Right},
        {"Space", sf::Keyboard::Scancode::Space},
        {"Enter", sf::Keyboard::Scancode::Enter},
        {"Tab", sf::Keyboard::Scancode::Tab},
        {"Backspace", sf::Keyboard::Scancode::Backspace},
        {"LShift", sf::Keyboard::Scancode::LShift},
        {"RShift", sf::Keyboard::Scancode::RShift},
        {"LControl", sf::Keyboard::Scancode::LControl},
        {"RControl", sf::Keyboard::Scancode::RControl},
        {"LAlt", sf::Keyboard::Scancode::LAlt},
        {"RAlt", sf::Keyboard::Scancode::RAlt},
    }};
    return named;
}

} // namespace engine::input
