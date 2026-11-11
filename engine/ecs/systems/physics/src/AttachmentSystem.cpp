#include "AttachmentSystem.hpp"

#include <cstddef>

#include "Attachment.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"

namespace engine::ecs {

AttachmentSystem::AttachmentSystem(Registry &registry) : m_registry(registry)
{
}

void AttachmentSystem::update(float)
{
    for (std::size_t i : m_registry.view<Attachment, Transform>()) {
        const Entity child = m_registry.entityAt(i);
        const Attachment &link = m_registry.get<Attachment>(child);
        const Entity parent = link.parent();
        if (!link.attached() || !m_registry.isAlive(parent) || !m_registry.has<Transform>(parent))
            continue;
        const Transform &anchor = m_registry.get<Transform>(parent);
        m_registry.get<Transform>(child).setPosition(anchor.x() + link.offsetX(), anchor.y() + link.offsetY());
        if (m_registry.has<Velocity>(child))
            m_registry.get<Velocity>(child).set(0.f, 0.f);
    }
}

} // namespace engine::ecs
