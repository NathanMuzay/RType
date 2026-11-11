#include "WorldValues.hpp"

#include <algorithm>
#include <string>

#include "EcsError.hpp"

namespace engine::ecs {

WorldValues::WorldValues(std::size_t count) : m_values(count, 0)
{
}

std::size_t WorldValues::size() const
{
    return m_values.size();
}

int WorldValues::value(std::size_t index) const
{
    if (index >= m_values.size())
        throw EcsError("WorldValues: no value at index " + std::to_string(index));
    return m_values[index];
}

void WorldValues::set(std::size_t index, int value)
{
    if (index >= m_values.size())
        throw EcsError("WorldValues: no value at index " + std::to_string(index));
    m_values[index] = static_cast<std::uint16_t>(std::clamp(value, 0, MAX_VALUE));
}

const std::vector<std::uint16_t> &WorldValues::values() const
{
    return m_values;
}

void WorldValues::reset()
{
    std::fill(m_values.begin(), m_values.end(), 0);
}

} // namespace engine::ecs
