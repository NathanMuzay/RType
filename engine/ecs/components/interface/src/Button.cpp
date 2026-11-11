#include "Button.hpp"

#include <utility>

namespace engine::ecs {

Button::Button(std::string action, float width, float height)
    : m_action(std::move(action)), m_width(width), m_height(height), m_normalColor(), m_hoveredColor(),
      m_pressedColor(), m_normalFrame(), m_hoveredFrame(), m_pressedFrame(), m_hoverSound(), m_clickSound(),
      m_wasHovered(false)
{
}

const std::string &Button::action() const
{
    return m_action;
}

void Button::setAction(std::string action)
{
    m_action = std::move(action);
}

float Button::width() const
{
    return m_width;
}

float Button::height() const
{
    return m_height;
}

bool Button::contains(Vector2 origin, Vector2 point) const
{
    return point.x() >= origin.x() && point.x() <= origin.x() + m_width && point.y() >= origin.y()
        && point.y() <= origin.y() + m_height;
}

void Button::setColors(Color normal, Color hovered, Color pressed)
{
    m_normalColor = normal;
    m_hoveredColor = hovered;
    m_pressedColor = pressed;
}

std::optional<Color> Button::colorFor(State state) const
{
    if (state == State::Pressed)
        return m_pressedColor;
    if (state == State::Hovered)
        return m_hoveredColor;
    return m_normalColor;
}

void Button::setFrames(IntRect normal, IntRect hovered, IntRect pressed)
{
    m_normalFrame = normal;
    m_hoveredFrame = hovered;
    m_pressedFrame = pressed;
}

std::optional<IntRect> Button::frameFor(State state) const
{
    if (state == State::Pressed)
        return m_pressedFrame;
    if (state == State::Hovered)
        return m_hoveredFrame;
    return m_normalFrame;
}

void Button::setSounds(std::string hoverSound, std::string clickSound)
{
    m_hoverSound = std::move(hoverSound);
    m_clickSound = std::move(clickSound);
}

const std::string &Button::hoverSound() const
{
    return m_hoverSound;
}

const std::string &Button::clickSound() const
{
    return m_clickSound;
}

bool Button::wasHovered() const
{
    return m_wasHovered;
}

void Button::setWasHovered(bool hovered)
{
    m_wasHovered = hovered;
}

} // namespace engine::ecs
