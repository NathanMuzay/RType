#include "DespawnOffscreen.hpp"

namespace engine::ecs {

DespawnOffscreen::DespawnOffscreen(float minX, float minY, float maxX, float maxY)
    : m_minX(minX), m_minY(minY), m_maxX(maxX), m_maxY(maxY)
{
}

bool DespawnOffscreen::isOutside(float x, float y, float width, float height) const
{
    return x + width <= m_minX || x >= m_maxX || y + height <= m_minY || y >= m_maxY;
}

} // namespace engine::ecs
