#include "Transform.hpp"

namespace engine::ecs {

Transform::Transform(float x, float y) : m_x(x), m_y(y)
{
}

float Transform::x() const
{
    return m_x;
}

float Transform::y() const
{
    return m_y;
}

void Transform::setPosition(float x, float y)
{
    m_x = x;
    m_y = y;
}

void Transform::translate(float dx, float dy)
{
    m_x += dx;
    m_y += dy;
}

} // namespace engine::ecs
