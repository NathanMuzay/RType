#include "Vector2.hpp"

namespace engine {

Vector2::Vector2(float x, float y) : m_x(x), m_y(y)
{
}

float Vector2::x() const
{
    return m_x;
}

float Vector2::y() const
{
    return m_y;
}

} // namespace engine
