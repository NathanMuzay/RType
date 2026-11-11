#include "Anchor.hpp"

namespace engine::ecs {

Anchor::Anchor(Entity target, float offsetX, float offsetY) : m_target(target), m_offsetX(offsetX), m_offsetY(offsetY)
{
}

Entity Anchor::target() const
{
    return m_target;
}

float Anchor::offsetX() const
{
    return m_offsetX;
}

float Anchor::offsetY() const
{
    return m_offsetY;
}

} // namespace engine::ecs
