#include "BoundsSystem.hpp"

#include <algorithm>

#include "Bounded.hpp"
#include "Collider.hpp"
#include "Transform.hpp"

namespace engine::ecs {

BoundsSystem::BoundsSystem(Registry &registry) : m_registry(registry)
{
}

void BoundsSystem::update(float)
{
    auto &transforms = m_registry.getComponents<Transform>();
    const auto &colliders = m_registry.getComponents<Collider>();
    const auto &bounds = m_registry.getComponents<Bounded>();
    for (std::size_t i : m_registry.view<Transform, Collider, Bounded>()) {
        Transform &transform = transforms.at(i);
        const Collider &collider = colliders.at(i);
        const Bounded &area = bounds.at(i);
        const float x = std::clamp(transform.x(), area.minX(), area.maxX() - collider.width());
        const float y = std::clamp(transform.y(), area.minY(), area.maxY() - collider.height());
        transform.setPosition(x, y);
    }
}

} // namespace engine::ecs
