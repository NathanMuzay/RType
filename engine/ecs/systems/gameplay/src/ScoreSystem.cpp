#include "ScoreSystem.hpp"

#include "Owner.hpp"
#include "Score.hpp"
#include "ScoreValue.hpp"

namespace engine::ecs {

ScoreSystem::ScoreSystem(Registry &registry, const EventQueue<DeathEvent> &deaths)
    : m_registry(registry), m_deaths(deaths)
{
}

void ScoreSystem::update(float)
{
    for (const DeathEvent &death : m_deaths.events()) {
        const Entity victim = death.victim();
        if (!death.killer() || !m_registry.isAlive(victim) || !m_registry.has<ScoreValue>(victim))
            continue;
        credit(*death.killer(), m_registry.get<ScoreValue>(victim).points());
    }
}

void ScoreSystem::credit(Entity killer, std::uint32_t points)
{
    for (std::size_t depth = 0; depth < MAX_OWNER_DEPTH; ++depth) {
        if (!m_registry.isAlive(killer))
            return;
        if (m_registry.has<Score>(killer)) {
            m_registry.get<Score>(killer).add(points);
            return;
        }
        if (!m_registry.has<Owner>(killer))
            return;
        killer = m_registry.get<Owner>(killer).owner();
    }
}

} // namespace engine::ecs
