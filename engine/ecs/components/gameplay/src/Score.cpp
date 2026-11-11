#include "Score.hpp"

namespace engine::ecs {

Score::Score() : m_points(0)
{
}

std::uint32_t Score::points() const
{
    return m_points;
}

void Score::setPoints(std::uint32_t points)
{
    m_points = points;
}

void Score::add(std::uint32_t points)
{
    m_points += points;
}

void Score::reset()
{
    m_points = 0;
}

} // namespace engine::ecs
