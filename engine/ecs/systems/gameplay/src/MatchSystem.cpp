#include "MatchSystem.hpp"

#include "Destroyed.hpp"
#include "Lives.hpp"
#include "Persistent.hpp"
#include "Player.hpp"
#include "PlayerValues.hpp"
#include "Respawn.hpp"
#include "Score.hpp"
#include "Transform.hpp"

namespace engine::ecs {

MatchSystem::MatchSystem(Registry &registry) : m_registry(registry)
{
}

void MatchSystem::update(float dt)
{
    const bool hasPlayers = !m_registry.view<Player>().empty();
    auto &matches = m_registry.getComponents<Match>();
    for (std::size_t i : m_registry.view<Match>()) {
        Match &match = matches.at(i);
        if (!hasPlayers) {
            if (match.phase() != Match::Phase::Waiting) {
                clearWorld();
                match.setPhase(Match::Phase::Waiting);
            }
            continue;
        }
        if (match.phase() == Match::Phase::Waiting) {
            resetPlayers();
            match.setPhase(Match::Phase::Playing);
        } else if (match.phase() == Match::Phase::Playing) {
            if (everyoneOut()) {
                match.setPhase(Match::Phase::Over);
                match.startTimer();
            }
        } else {
            match.elapse(dt);
            if (match.timer() <= 0.f) {
                clearWorld();
                resetPlayers();
                match.setPhase(Match::Phase::Playing);
            }
        }
    }
}

Match::Phase MatchSystem::phaseOf(const Registry &registry)
{
    const auto matches = registry.view<Match>();
    if (matches.empty())
        return Match::Phase::Playing;
    return registry.getComponents<Match>().at(matches.front()).phase();
}

bool MatchSystem::everyoneOut() const
{
    for (std::size_t i : m_registry.view<Player>()) {
        const Entity owner = m_registry.entityAt(i);
        if (!m_registry.has<Lives>(owner) || m_registry.get<Lives>(owner).count() > 0)
            return false;
        const auto avatar = m_registry.get<Player>(owner).avatar();
        if (avatar && Destroyed::isActive(m_registry, *avatar))
            return false;
    }
    return true;
}

void MatchSystem::clearWorld()
{
    for (std::size_t i : m_registry.view<Transform>()) {
        const Entity entity = m_registry.entityAt(i);
        if (!m_registry.has<Persistent>(entity))
            m_registry.kill(entity);
    }
}

void MatchSystem::resetPlayers()
{
    for (std::size_t i : m_registry.view<Player>()) {
        const Entity owner = m_registry.entityAt(i);
        if (m_registry.has<Lives>(owner))
            m_registry.get<Lives>(owner).reset();
        if (m_registry.has<Score>(owner))
            m_registry.get<Score>(owner).reset();
        if (m_registry.has<PlayerValues>(owner))
            m_registry.get<PlayerValues>(owner).reset();
        if (m_registry.has<Respawn>(owner))
            m_registry.get<Respawn>(owner).reset();
        m_registry.get<Player>(owner).clearAvatar();
    }
}

} // namespace engine::ecs
