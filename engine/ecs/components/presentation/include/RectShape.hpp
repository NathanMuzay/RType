#pragma once

#include "Color.hpp"
#include "IComponent.hpp"

namespace engine::ecs {

class RectShape : public IComponent
{
public:
    RectShape(float width, float height, Color color, Color outline = Color(0, 0, 0, 0), float thickness = 0.f);

    float width() const;
    float height() const;
    Color color() const;
    Color outline() const;
    float thickness() const;
    void setColor(Color color);
    void setWidth(float width);
    void setHeight(float height);

private:
    float m_width;
    float m_height;
    Color m_color;
    Color m_outline;
    float m_thickness;
};

} // namespace engine::ecs
