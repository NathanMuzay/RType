#include "Collider.hpp"

namespace engine::ecs {

Collider::Collider(float width, float height, std::uint32_t layer, std::uint32_t hitMask)
    : m_width(width), m_height(height), m_layer(layer), m_hitMask(hitMask)
{
}

float Collider::width() const
{
    return m_width;
}

float Collider::height() const
{
    return m_height;
}

std::uint32_t Collider::layer() const
{
    return m_layer;
}

bool Collider::canHit(const Collider &other) const
{
    return (m_hitMask & other.m_layer) != 0;
}

physics::Aabb Collider::boundsAt(float x, float y) const
{
    return physics::Aabb(x, y, m_width, m_height);
}

} // namespace engine::ecs
