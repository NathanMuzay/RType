#include "LivesSystem.hpp"

#include "Lives.hpp"
#include "Player.hpp"

namespace engine::ecs {

LivesSystem::LivesSystem(Registry &registry, const EventQueue<DeathEvent> &deaths)
    : m_registry(registry), m_deaths(deaths)
{
}

void LivesSystem::update(float)
{
    const auto &players = m_registry.getComponents<Player>();
    auto &lives = m_registry.getComponents<Lives>();
    for (const DeathEvent &death : m_deaths.events()) {
        for (std::size_t i : m_registry.view<Player, Lives>()) {
            if (players.at(i).avatar() == death.victim())
                lives.at(i).lose();
        }
    }
}

} // namespace engine::ecs
