#include "RectShape.hpp"

namespace engine::ecs {

RectShape::RectShape(float width, float height, Color color, Color outline, float thickness)
    : m_width(width), m_height(height), m_color(color), m_outline(outline), m_thickness(thickness)
{
}

float RectShape::width() const
{
    return m_width;
}

float RectShape::height() const
{
    return m_height;
}

Color RectShape::color() const
{
    return m_color;
}

Color RectShape::outline() const
{
    return m_outline;
}

float RectShape::thickness() const
{
    return m_thickness;
}

void RectShape::setColor(Color color)
{
    m_color = color;
}

void RectShape::setWidth(float width)
{
    m_width = width;
}

void RectShape::setHeight(float height)
{
    m_height = height;
}

} // namespace engine::ecs
