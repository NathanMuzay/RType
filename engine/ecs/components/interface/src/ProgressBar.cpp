#include "ProgressBar.hpp"

#include <utility>

namespace engine::ecs {

ProgressBar::ProgressBar(std::string key, float width, float maximum, bool vertical)
    : m_key(std::move(key)), m_width(width), m_maximum(maximum), m_vertical(vertical)
{
}

const std::string &ProgressBar::key() const
{
    return m_key;
}

float ProgressBar::width() const
{
    return m_width;
}

float ProgressBar::maximum() const
{
    return m_maximum;
}

bool ProgressBar::vertical() const
{
    return m_vertical;
}

} // namespace engine::ecs
