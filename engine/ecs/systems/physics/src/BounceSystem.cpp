#include "BounceSystem.hpp"

#include "Bounce.hpp"
#include "Bounded.hpp"
#include "Collider.hpp"
#include "Solid.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"

namespace engine::ecs {

BounceSystem::BounceSystem(Registry &registry) : m_registry(registry)
{
}

void BounceSystem::update(float)
{
    const auto walls = m_registry.view<Solid, Transform, Collider>();
    for (std::size_t i : m_registry.view<Bounce, Transform, Velocity, Collider>()) {
        if (m_registry.has<Bounded>(m_registry.entityAt(i)))
            bounceOnBounds(i);
        for (std::size_t wall : walls) {
            if (wall != i)
                bounceOnSolid(i, wall);
        }
    }
}

void BounceSystem::bounceOnBounds(std::size_t index)
{
    const Entity entity = m_registry.entityAt(index);
    const Transform &transform = m_registry.get<Transform>(entity);
    const Collider &collider = m_registry.get<Collider>(entity);
    const Bounded &area = m_registry.get<Bounded>(entity);
    const float restitution = m_registry.get<Bounce>(entity).restitution();
    Velocity &velocity = m_registry.get<Velocity>(entity);
    if ((transform.x() <= area.minX() && velocity.x() < 0.f)
        || (transform.x() + collider.width() >= area.maxX() && velocity.x() > 0.f))
        velocity.setX(-velocity.x() * restitution);
    if ((transform.y() <= area.minY() && velocity.y() < 0.f)
        || (transform.y() + collider.height() >= area.maxY() && velocity.y() > 0.f))
        velocity.setY(-velocity.y() * restitution);
}

void BounceSystem::bounceOnSolid(std::size_t index, std::size_t wall)
{
    auto &transforms = m_registry.getComponents<Transform>();
    const auto &colliders = m_registry.getComponents<Collider>();
    Transform &body = transforms.at(index);
    const Transform &block = transforms.at(wall);
    const auto push =
        colliders.at(index).boundsAt(body.x(), body.y()).separation(colliders.at(wall).boundsAt(block.x(), block.y()));
    if (!push)
        return;
    const Entity entity = m_registry.entityAt(index);
    const float restitution = m_registry.get<Bounce>(entity).restitution();
    Velocity &velocity = m_registry.get<Velocity>(entity);
    body.translate(push->first, push->second);
    if (push->first != 0.f)
        velocity.setX(-velocity.x() * restitution);
    else
        velocity.setY(-velocity.y() * restitution);
}

} // namespace engine::ecs
