#include "PlayerLabel.hpp"

#include <utility>

namespace engine::ecs {

PlayerLabel::PlayerLabel(
    std::string font, std::string format, unsigned int size, float offsetX, float offsetY, std::vector<Color> colors)
    : m_font(std::move(font)), m_format(std::move(format)), m_size(size), m_offsetX(offsetX), m_offsetY(offsetY),
      m_colors(std::move(colors)), m_attached(false)
{
}

const std::string &PlayerLabel::font() const
{
    return m_font;
}

const std::string &PlayerLabel::format() const
{
    return m_format;
}

unsigned int PlayerLabel::size() const
{
    return m_size;
}

float PlayerLabel::offsetX() const
{
    return m_offsetX;
}

float PlayerLabel::offsetY() const
{
    return m_offsetY;
}

Color PlayerLabel::colorFor(std::size_t slot) const
{
    if (m_colors.empty())
        return Color();
    return m_colors[slot % m_colors.size()];
}

bool PlayerLabel::attached() const
{
    return m_attached;
}

void PlayerLabel::markAttached()
{
    m_attached = true;
}

} // namespace engine::ecs
