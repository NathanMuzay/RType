#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string>

#include "UiKey.hpp"
#include "Vector2.hpp"

namespace engine {

class UiInput
{
public:
    static constexpr std::size_t KEY_COUNT = static_cast<std::size_t>(UiKey::Right) + 1;
    static constexpr char FIRST_PRINTABLE = ' ';
    static constexpr char LAST_PRINTABLE = '~';

    UiInput();

    void beginFrame();
    void movePointer(Vector2 position);
    void pressPointer(Vector2 position);
    void releasePointer();
    void typeCharacter(char character);
    void pressKey(UiKey key);
    void pressRawKey(int code);
    void pressRawButton(unsigned int button);

    Vector2 pointer() const;
    bool pointerMoved() const;
    bool clicked() const;
    bool pointerDown() const;
    const std::string &typedText() const;
    bool keyPressed(UiKey key) const;
    std::optional<int> rawKey() const;
    std::optional<unsigned int> rawButton() const;

private:
    Vector2 m_pointer;
    bool m_pointerMoved;
    bool m_clicked;
    bool m_pointerDown;
    std::string m_typedText;
    std::array<bool, KEY_COUNT> m_keys;
    std::optional<int> m_rawKey;
    std::optional<unsigned int> m_rawButton;
};

} // namespace engine
