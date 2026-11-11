#include "GravitySystem.hpp"

#include <cstddef>

#include "Gravity.hpp"
#include "Velocity.hpp"

namespace engine::ecs {

GravitySystem::GravitySystem(Registry &registry) : m_registry(registry)
{
}

void GravitySystem::update(float dt)
{
    const auto &gravities = m_registry.getComponents<Gravity>();
    auto &velocities = m_registry.getComponents<Velocity>();
    for (std::size_t i : m_registry.view<Gravity, Velocity>()) {
        Velocity &velocity = velocities.at(i);
        velocity.setY(velocity.y() + gravities.at(i).force() * dt);
    }
}

} // namespace engine::ecs
