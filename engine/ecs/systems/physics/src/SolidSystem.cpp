#include "SolidSystem.hpp"

#include "Collider.hpp"
#include "Pushable.hpp"
#include "Solid.hpp"
#include "Transform.hpp"

namespace engine::ecs {

SolidSystem::SolidSystem(Registry &registry) : m_registry(registry)
{
}

void SolidSystem::update(float)
{
    const auto walls = m_registry.view<Solid, Transform, Collider>();
    for (std::size_t mover : m_registry.view<Pushable, Transform, Collider>()) {
        for (std::size_t wall : walls)
            pushOut(mover, wall);
    }
}

void SolidSystem::pushOut(std::size_t mover, std::size_t wall)
{
    auto &transforms = m_registry.getComponents<Transform>();
    const auto &colliders = m_registry.getComponents<Collider>();
    Transform &body = transforms.at(mover);
    const Transform &block = transforms.at(wall);
    const auto push =
        colliders.at(mover).boundsAt(body.x(), body.y()).separation(colliders.at(wall).boundsAt(block.x(), block.y()));
    if (push)
        body.translate(push->first, push->second);
}

} // namespace engine::ecs
