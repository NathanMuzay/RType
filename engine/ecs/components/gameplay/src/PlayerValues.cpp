#include "PlayerValues.hpp"

#include <algorithm>

namespace engine::ecs {

PlayerValues::PlayerValues(std::size_t count) : m_values(count, 0)
{
}

const std::vector<std::uint16_t> &PlayerValues::values() const
{
    return m_values;
}

void PlayerValues::set(std::size_t index, int value)
{
    if (index < m_values.size())
        m_values[index] = static_cast<std::uint16_t>(std::clamp(value, 0, MAX_VALUE));
}

void PlayerValues::reset()
{
    std::fill(m_values.begin(), m_values.end(), 0);
}

} // namespace engine::ecs
