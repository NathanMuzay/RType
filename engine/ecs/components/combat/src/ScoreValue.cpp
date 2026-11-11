#include "ScoreValue.hpp"

namespace engine::ecs {

ScoreValue::ScoreValue(std::uint32_t points) : m_points(points)
{
}

std::uint32_t ScoreValue::points() const
{
    return m_points;
}

} // namespace engine::ecs
