#include "AnchorSystem.hpp"

#include "Anchor.hpp"
#include "Destroyed.hpp"
#include "Transform.hpp"

namespace engine::ecs {

AnchorSystem::AnchorSystem(Registry &registry) : m_registry(registry)
{
}

void AnchorSystem::update(float)
{
    for (std::size_t i : m_registry.view<Anchor, Transform>()) {
        const Entity follower = m_registry.entityAt(i);
        const Anchor &anchor = m_registry.get<Anchor>(follower);
        const Entity target = anchor.target();
        if (!m_registry.isAlive(target) || !m_registry.has<Transform>(target)) {
            if (!m_registry.has<Destroyed>(follower))
                m_registry.emplace<Destroyed>(follower);
            continue;
        }
        const Transform &position = m_registry.get<Transform>(target);
        m_registry.get<Transform>(follower).setPosition(
            position.x() + anchor.offsetX(), position.y() + anchor.offsetY());
    }
}

} // namespace engine::ecs
