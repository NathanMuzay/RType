#pragma once

#include <optional>
#include <string>

#include "Color.hpp"
#include "IComponent.hpp"
#include "IntRect.hpp"
#include "Vector2.hpp"

namespace engine::ecs {

class Button : public IComponent
{
public:
    enum class State
    {
        Normal,
        Hovered,
        Pressed
    };

    Button(std::string action, float width, float height);

    const std::string &action() const;
    void setAction(std::string action);
    float width() const;
    float height() const;
    bool contains(Vector2 origin, Vector2 point) const;

    void setColors(Color normal, Color hovered, Color pressed);
    std::optional<Color> colorFor(State state) const;
    void setFrames(IntRect normal, IntRect hovered, IntRect pressed);
    std::optional<IntRect> frameFor(State state) const;
    void setSounds(std::string hoverSound, std::string clickSound);
    const std::string &hoverSound() const;
    const std::string &clickSound() const;

    bool wasHovered() const;
    void setWasHovered(bool hovered);

private:
    std::string m_action;
    float m_width;
    float m_height;
    std::optional<Color> m_normalColor;
    std::optional<Color> m_hoveredColor;
    std::optional<Color> m_pressedColor;
    std::optional<IntRect> m_normalFrame;
    std::optional<IntRect> m_hoveredFrame;
    std::optional<IntRect> m_pressedFrame;
    std::string m_hoverSound;
    std::string m_clickSound;
    bool m_wasHovered;
};

} // namespace engine::ecs
