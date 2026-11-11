#include "CollisionSystem.hpp"

#include <vector>

#include "Collider.hpp"
#include "Collision.hpp"
#include "Transform.hpp"

namespace engine::ecs {

CollisionSystem::CollisionSystem(Registry &registry, EventQueue<CollisionEvent> &collisions)
    : m_registry(registry), m_collisions(collisions)
{
}

void CollisionSystem::update(float)
{
    const auto &transforms = m_registry.getComponents<Transform>();
    const auto &colliders = m_registry.getComponents<Collider>();
    std::vector<physics::Body> bodies;
    for (std::size_t i : m_registry.view<Transform, Collider>()) {
        const Transform &transform = transforms.at(i);
        const Collider &collider = colliders.at(i);
        bodies.emplace_back(i, collider.boundsAt(transform.x(), transform.y()));
    }
    for (const physics::CollisionPair &pair : physics::findCollisions(bodies)) {
        const Collider &first = colliders.at(pair.first());
        const Collider &second = colliders.at(pair.second());
        if (first.canHit(second) || second.canHit(first))
            m_collisions.push(CollisionEvent(m_registry.entityAt(pair.first()), m_registry.entityAt(pair.second())));
    }
}

} // namespace engine::ecs
