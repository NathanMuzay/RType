#pragma once

#include <cstdint>

#include "Aabb.hpp"
#include "IComponent.hpp"

namespace engine::ecs {

class Collider : public IComponent
{
public:
    Collider(float width, float height, std::uint32_t layer, std::uint32_t hitMask);

    float width() const;
    float height() const;
    std::uint32_t layer() const;
    bool canHit(const Collider &other) const;
    physics::Aabb boundsAt(float x, float y) const;

private:
    float m_width;
    float m_height;
    std::uint32_t m_layer;
    std::uint32_t m_hitMask;
};

} // namespace engine::ecs
