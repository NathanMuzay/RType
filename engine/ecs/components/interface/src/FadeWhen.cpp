#include "FadeWhen.hpp"

#include <algorithm>
#include <utility>

namespace engine::ecs {

FadeWhen::FadeWhen(std::string key, std::string value, float seconds)
    : m_key(std::move(key)), m_value(std::move(value)), m_seconds(seconds), m_opacity(0.f)
{
}

const std::string &FadeWhen::key() const
{
    return m_key;
}

const std::string &FadeWhen::value() const
{
    return m_value;
}

float FadeWhen::opacity() const
{
    return m_opacity;
}

void FadeWhen::fade(bool visible, float dt)
{
    const float step = m_seconds > 0.f ? dt / m_seconds : 1.f;
    m_opacity = std::clamp(m_opacity + (visible ? step : -step), 0.f, 1.f);
}

} // namespace engine::ecs
