#include "OffscreenSystem.hpp"

#include "Collider.hpp"
#include "DespawnOffscreen.hpp"
#include "Destroyed.hpp"
#include "Transform.hpp"

namespace engine::ecs {

OffscreenSystem::OffscreenSystem(Registry &registry) : m_registry(registry)
{
}

void OffscreenSystem::update(float)
{
    const auto &transforms = m_registry.getComponents<Transform>();
    const auto &colliders = m_registry.getComponents<Collider>();
    const auto &areas = m_registry.getComponents<DespawnOffscreen>();
    for (std::size_t i : m_registry.view<Transform, Collider, DespawnOffscreen>()) {
        const Transform &transform = transforms.at(i);
        const Collider &collider = colliders.at(i);
        const Entity entity = m_registry.entityAt(i);
        if (areas.at(i).isOutside(transform.x(), transform.y(), collider.width(), collider.height())
            && !m_registry.has<Destroyed>(entity))
            m_registry.emplace<Destroyed>(entity);
    }
}

} // namespace engine::ecs
