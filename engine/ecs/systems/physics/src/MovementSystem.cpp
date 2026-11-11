#include "MovementSystem.hpp"

#include "Transform.hpp"
#include "Velocity.hpp"

namespace engine::ecs {

MovementSystem::MovementSystem(Registry &registry) : m_registry(registry)
{
}

void MovementSystem::update(float dt)
{
    auto &transforms = m_registry.getComponents<Transform>();
    const auto &velocities = m_registry.getComponents<Velocity>();
    for (std::size_t i : m_registry.view<Transform, Velocity>())
        transforms.at(i).translate(velocities.at(i).x() * dt, velocities.at(i).y() * dt);
}

} // namespace engine::ecs
