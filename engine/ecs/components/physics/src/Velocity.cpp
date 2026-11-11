#include "Velocity.hpp"

namespace engine::ecs {

Velocity::Velocity(float x, float y) : m_x(x), m_y(y)
{
}

float Velocity::x() const
{
    return m_x;
}

float Velocity::y() const
{
    return m_y;
}

void Velocity::set(float x, float y)
{
    m_x = x;
    m_y = y;
}

void Velocity::setX(float x)
{
    m_x = x;
}

void Velocity::setY(float y)
{
    m_y = y;
}

} // namespace engine::ecs
