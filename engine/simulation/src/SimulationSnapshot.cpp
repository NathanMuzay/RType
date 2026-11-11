#include "Simulation.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <utility>

#include "ComponentRegistration.hpp"
#include "ConsoleTextMessage.hpp"
#include "Destroyed.hpp"
#include "EngineComponentLoaders.hpp"
#include "EngineSystemBuilders.hpp"
#include "Health.hpp"
#include "HitFlash.hpp"
#include "Lives.hpp"
#include "MatchSystem.hpp"
#include "Player.hpp"
#include "PlayerControl.hpp"
#include "PlayerValues.hpp"
#include "PlaySoundMessage.hpp"
#include "Protocol.hpp"
#include "Replicated.hpp"
#include "Rotation.hpp"
#include "SceneBuilder.hpp"
#include "Score.hpp"
#include "SimulationError.hpp"
#include "SoundEvent.hpp"
#include "Transform.hpp"

namespace engine::simulation {

network::SnapshotMessage Simulation::snapshot() const
{
    network::SnapshotMessage::Players players;
    players.reserve(m_settings.maxPlayers());
    for (std::size_t slot = 0; slot < m_settings.maxPlayers(); ++slot)
        players.push_back(statusOf(slot));
    return network::SnapshotMessage(
        m_tick, phaseCode(phase()), m_worldValues.values(), std::move(players), collectEntities());
}

network::PlayerStatus Simulation::statusOf(std::size_t slot) const
{
    const auto player = playerEntity(slot);
    if (!player)
        return network::PlayerStatus(
            network::Protocol::SLOT_EMPTY, 0, 0, 0, 0, std::vector<std::uint16_t>(m_playerTable.size(), 0));
    const int lives = m_registry.has<ecs::Lives>(*player) ? m_registry.get<ecs::Lives>(*player).count() : 0;
    const std::uint32_t score = m_registry.has<ecs::Score>(*player) ? m_registry.get<ecs::Score>(*player).points() : 0;
    const auto avatar = avatarOf(slot);
    const std::uint16_t avatarId = avatar ? networkIdOf(*avatar) : 0;
    const std::uint8_t state = avatarId != 0 ? network::Protocol::SLOT_ALIVE : network::Protocol::SLOT_DEAD;
    const std::uint8_t health = avatar ? healthPercent(*avatar) : 0;
    std::vector<std::uint16_t> values(m_playerTable.size(), 0);
    if (m_registry.has<ecs::PlayerValues>(*player))
        values = m_registry.get<ecs::PlayerValues>(*player).values();
    return network::PlayerStatus(state, static_cast<std::uint8_t>(std::clamp(lives, 0, MAX_REPORTED_LIVES)), health,
        score, avatarId, std::move(values));
}

std::uint8_t Simulation::healthPercent(const ecs::Entity &entity) const
{
    if (!m_registry.has<ecs::Health>(entity))
        return 0;
    const ecs::Health &health = m_registry.get<ecs::Health>(entity);
    if (health.maximum() <= 0)
        return 0;
    return percent(static_cast<float>(health.points()) / static_cast<float>(health.maximum()));
}

std::uint8_t Simulation::percent(float ratio)
{
    const float clamped = std::clamp(ratio, 0.f, 1.f);
    return static_cast<std::uint8_t>(std::lround(clamped * network::Protocol::MAX_PERCENT));
}

std::vector<network::EntityState> Simulation::collectEntities() const
{
    std::vector<network::EntityState> entities;
    std::vector<ecs::Entity> avatars;
    for (std::size_t slot = 0; slot < m_settings.maxPlayers(); ++slot) {
        const auto avatar = avatarOf(slot);
        if (!avatar)
            continue;
        avatars.push_back(*avatar);
        if (const auto described = describe(*avatar))
            entities.push_back(*described);
    }
    for (std::size_t i : m_registry.view<ecs::Transform, ecs::Replicated>()) {
        if (entities.size() >= m_settings.maxEntities())
            break;
        const ecs::Entity entity = m_registry.entityAt(i);
        if (std::find(avatars.begin(), avatars.end(), entity) != avatars.end())
            continue;
        if (const auto described = describe(entity))
            entities.push_back(*described);
    }
    return entities;
}

std::optional<network::EntityState> Simulation::describe(const ecs::Entity &entity) const
{
    if (!m_registry.has<ecs::Transform>(entity) || !m_registry.has<ecs::Replicated>(entity))
        return std::nullopt;
    const ecs::Replicated &identity = m_registry.get<ecs::Replicated>(entity);
    if (identity.networkId() == 0)
        return std::nullopt;
    const ecs::Transform &transform = m_registry.get<ecs::Transform>(entity);
    return network::EntityState(identity.networkId(), identity.kind(),
        static_cast<std::int16_t>(std::lround(transform.x())), static_cast<std::int16_t>(std::lround(transform.y())),
        visualOf(entity));
}

std::uint16_t Simulation::networkIdOf(const ecs::Entity &entity) const
{
    if (!m_registry.isAlive(entity) || !m_registry.has<ecs::Replicated>(entity))
        return 0;
    return m_registry.get<ecs::Replicated>(entity).networkId();
}

std::uint8_t Simulation::phaseCode(ecs::Match::Phase phase)
{
    if (phase == ecs::Match::Phase::Playing)
        return network::Protocol::GAME_PLAYING;
    if (phase == ecs::Match::Phase::Over)
        return network::Protocol::GAME_OVER;
    return network::Protocol::GAME_WAITING;
}

std::uint8_t Simulation::visualOf(const ecs::Entity &entity) const
{
    std::uint8_t visual = 0;
    if (m_registry.has<ecs::HitFlash>(entity))
        visual |= network::Protocol::VISUAL_FLASH;
    if (m_registry.has<ecs::Rotation>(entity)) {
        const float degrees =
            std::fmod(std::fmod(m_registry.get<ecs::Rotation>(entity).degrees(), 360.f) + 360.f, 360.f);
        const auto step = static_cast<int>(std::lround(degrees / 360.f * network::Protocol::VISUAL_ANGLE_STEPS));
        visual |= static_cast<std::uint8_t>(step) & network::Protocol::VISUAL_ANGLE_MASK;
    }
    return visual;
}

} // namespace engine::simulation
