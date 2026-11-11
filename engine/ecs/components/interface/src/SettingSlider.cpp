#include "SettingSlider.hpp"

#include <cmath>
#include <cstdlib>
#include <utility>

namespace engine::ecs {

SettingSlider::SettingSlider(std::string setting, std::vector<std::string> values, bool wrap, int direction)
    : m_setting(std::move(setting)), m_values(std::move(values)), m_wrap(wrap), m_direction(direction)
{
}

int SettingSlider::direction() const
{
    return m_direction;
}

std::string SettingSlider::storeKey() const
{
    return STORE_PREFIX + m_setting;
}

std::string SettingSlider::step(const std::string &current, int direction) const
{
    if (m_values.empty())
        return current;
    const auto count = static_cast<long>(m_values.size());
    long index = static_cast<long>(indexOf(current)) + direction;
    if (m_wrap)
        index = (index % count + count) % count;
    else if (index < 0)
        index = 0;
    else if (index >= count)
        index = count - 1;
    return m_values[static_cast<std::size_t>(index)];
}

std::size_t SettingSlider::indexOf(const std::string &current) const
{
    for (std::size_t index = 0; index < m_values.size(); ++index) {
        if (m_values[index] == current)
            return index;
    }
    const float number = std::strtof(current.c_str(), nullptr);
    std::size_t closest = 0;
    float best = -1.f;
    for (std::size_t index = 0; index < m_values.size(); ++index) {
        const float distance = std::abs(std::strtof(m_values[index].c_str(), nullptr) - number);
        if (best < 0.f || distance < best) {
            best = distance;
            closest = index;
        }
    }
    return closest;
}

} // namespace engine::ecs
