#include "Aabb.hpp"

#include <algorithm>

namespace engine::physics {

Aabb::Aabb(float left, float top, float width, float height)
    : m_left(left), m_top(top), m_width(width), m_height(height)
{
}

float Aabb::left() const
{
    return m_left;
}

float Aabb::width() const
{
    return m_width;
}

float Aabb::height() const
{
    return m_height;
}

bool Aabb::overlaps(const Aabb &other) const
{
    return m_left < other.m_left + other.m_width && other.m_left < m_left + m_width
        && m_top < other.m_top + other.m_height && other.m_top < m_top + m_height;
}

std::optional<std::pair<float, float>> Aabb::separation(const Aabb &other) const
{
    const float left = other.m_left + other.m_width - m_left;
    const float right = m_left + m_width - other.m_left;
    const float up = other.m_top + other.m_height - m_top;
    const float down = m_top + m_height - other.m_top;
    if (left <= 0.f || right <= 0.f || up <= 0.f || down <= 0.f)
        return std::nullopt;
    const float smallest = std::min({left, right, up, down});
    if (smallest == left)
        return std::make_pair(left, 0.f);
    if (smallest == right)
        return std::make_pair(-right, 0.f);
    if (smallest == up)
        return std::make_pair(0.f, up);
    return std::make_pair(0.f, -down);
}

} // namespace engine::physics
