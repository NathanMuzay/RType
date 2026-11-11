#include "PlayerControlSystem.hpp"

#include "PlayerControl.hpp"
#include "Velocity.hpp"

namespace engine::ecs {

PlayerControlSystem::PlayerControlSystem(Registry &registry) : m_registry(registry)
{
}

void PlayerControlSystem::update(float dt)
{
    auto &controls = m_registry.getComponents<PlayerControl>();
    for (std::size_t i : m_registry.view<PlayerControl>())
        controls.at(i).advance(dt);
    auto &velocities = m_registry.getComponents<Velocity>();
    for (std::size_t i : m_registry.view<PlayerControl, Velocity>()) {
        const PlayerControl &control = controls.at(i);
        velocities.at(i).set(control.horizontal() * control.speed(), control.vertical() * control.speed());
    }
}

} // namespace engine::ecs
