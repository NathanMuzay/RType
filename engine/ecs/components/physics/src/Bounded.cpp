#include "Bounded.hpp"

namespace engine::ecs {

Bounded::Bounded(float minX, float minY, float maxX, float maxY)
    : m_minX(minX), m_minY(minY), m_maxX(maxX), m_maxY(maxY)
{
}

float Bounded::minX() const
{
    return m_minX;
}

float Bounded::minY() const
{
    return m_minY;
}

float Bounded::maxX() const
{
    return m_maxX;
}

float Bounded::maxY() const
{
    return m_maxY;
}

} // namespace engine::ecs
