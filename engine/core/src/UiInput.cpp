#include "UiInput.hpp"

namespace engine {

UiInput::UiInput()
    : m_pointer(), m_pointerMoved(false), m_clicked(false), m_pointerDown(false), m_typedText(), m_keys(), m_rawKey(),
      m_rawButton()
{
}

void UiInput::beginFrame()
{
    m_pointerMoved = false;
    m_clicked = false;
    m_typedText.clear();
    m_keys.fill(false);
    m_rawKey.reset();
    m_rawButton.reset();
}

void UiInput::pressRawKey(int code)
{
    m_rawKey = code;
}

void UiInput::pressRawButton(unsigned int button)
{
    m_rawButton = button;
}

std::optional<int> UiInput::rawKey() const
{
    return m_rawKey;
}

std::optional<unsigned int> UiInput::rawButton() const
{
    return m_rawButton;
}

void UiInput::movePointer(Vector2 position)
{
    m_pointer = position;
    m_pointerMoved = true;
}

void UiInput::pressPointer(Vector2 position)
{
    movePointer(position);
    m_clicked = true;
    m_pointerDown = true;
}

void UiInput::releasePointer()
{
    m_pointerDown = false;
}

void UiInput::typeCharacter(char character)
{
    if (character >= FIRST_PRINTABLE && character <= LAST_PRINTABLE)
        m_typedText.push_back(character);
}

void UiInput::pressKey(UiKey key)
{
    m_keys[static_cast<std::size_t>(key)] = true;
}

Vector2 UiInput::pointer() const
{
    return m_pointer;
}

bool UiInput::pointerMoved() const
{
    return m_pointerMoved;
}

bool UiInput::clicked() const
{
    return m_clicked;
}

bool UiInput::pointerDown() const
{
    return m_pointerDown;
}

const std::string &UiInput::typedText() const
{
    return m_typedText;
}

bool UiInput::keyPressed(UiKey key) const
{
    return m_keys[static_cast<std::size_t>(key)];
}

} // namespace engine
