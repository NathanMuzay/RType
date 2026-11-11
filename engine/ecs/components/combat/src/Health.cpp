#include "Health.hpp"

#include <algorithm>

namespace engine::ecs {

Health::Health(int points) : m_points(points), m_maximum(points)
{
}

int Health::points() const
{
    return m_points;
}

void Health::damage(int amount)
{
    m_points -= amount;
}

bool Health::isDead() const
{
    return m_points <= 0;
}

int Health::maximum() const
{
    return m_maximum;
}

void Health::heal(int amount)
{
    m_points = std::min(m_maximum, m_points + amount);
}

void Health::set(int points, int maximum)
{
    m_maximum = maximum;
    m_points = std::min(points, maximum);
}

} // namespace engine::ecs
