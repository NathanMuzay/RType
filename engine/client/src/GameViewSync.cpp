#include "GameView.hpp"

#include <algorithm>
#include <charconv>
#include <system_error>

#include "Button.hpp"
#include "ClientError.hpp"
#include "ComponentRegistration.hpp"
#include "DeathEvent.hpp"
#include "Destroyed.hpp"
#include "EngineComponentLoaders.hpp"
#include "EngineSystemBuilders.hpp"
#include "HitFlash.hpp"
#include "Lives.hpp"
#include "Persistent.hpp"
#include "Player.hpp"
#include "PlayerIndex.hpp"
#include "Protocol.hpp"
#include "Rotation.hpp"
#include "Score.hpp"
#include "TextField.hpp"
#include "TiltAnimation.hpp"
#include "Transform.hpp"
#include "UiFocus.hpp"

namespace engine::client {

void GameView::beginSync()
{
    m_seen.clear();
}

void GameView::syncEntity(std::uint16_t networkId, std::uint8_t kind, float x, float y, std::uint8_t visual)
{
    const auto prefab = m_kindPrefabs.find(kind);
    if (prefab == m_kindPrefabs.end())
        return;
    m_seen.insert(networkId);
    auto found = m_networkEntities.find(networkId);
    if (found == m_networkEntities.end())
        found = m_networkEntities.emplace(networkId, createRemote(prefab->second, x, y)).first;
    const ecs::Entity entity = found->second;
    if (!m_registry.isAlive(entity) || !m_registry.has<ecs::Transform>(entity))
        return;
    ecs::Transform &transform = m_registry.get<ecs::Transform>(entity);
    if (m_registry.has<ecs::TiltAnimation>(entity))
        m_registry.get<ecs::TiltAnimation>(entity).setDirection(verticalDirection(transform.y(), y));
    transform.setPosition(x, y);
    applyVisual(entity, visual);
}

void GameView::endSync()
{
    for (auto it = m_networkEntities.begin(); it != m_networkEntities.end();) {
        if (m_seen.contains(it->first)) {
            ++it;
            continue;
        }
        m_registry.kill(it->second);
        it = m_networkEntities.erase(it);
    }
}

void GameView::destroyEntity(std::uint16_t networkId)
{
    const auto found = m_networkEntities.find(networkId);
    if (found == m_networkEntities.end())
        return;
    const ecs::Entity entity = found->second;
    m_networkEntities.erase(found);
    if (!m_registry.has<ecs::Destroyed>(entity))
        m_registry.emplace<ecs::Destroyed>(entity);
    m_context.deaths().push(ecs::DeathEvent(entity, std::nullopt));
}

void GameView::setPhase(ecs::Match::Phase phase)
{
    m_registry.get<ecs::Match>(*m_match).setPhase(phase);
}

void GameView::setPlayer(std::size_t slot, int lives, std::uint32_t score, std::uint16_t avatarId)
{
    std::optional<ecs::Entity> player = playerEntity(slot);
    if (!player) {
        player = m_registry.spawn();
        m_registry.emplace<ecs::Player>(*player, slot);
        m_registry.emplace<ecs::Lives>(*player, lives);
        m_registry.emplace<ecs::Score>(*player);
        m_registry.emplace<ecs::Persistent>(*player);
    }
    m_registry.get<ecs::Lives>(*player).setCount(lives);
    m_registry.get<ecs::Score>(*player).setPoints(score);
    const auto avatar = m_networkEntities.find(avatarId);
    if (avatarId != 0 && avatar != m_networkEntities.end() && !m_registry.has<ecs::PlayerIndex>(avatar->second))
        m_registry.emplace<ecs::PlayerIndex>(avatar->second, slot);
}

void GameView::removePlayer(std::size_t slot)
{
    if (const auto player = playerEntity(slot))
        m_registry.kill(*player);
}

void GameView::clearRemoteWorld()
{
    for (const auto &entry : m_networkEntities)
        m_registry.kill(entry.second);
    m_networkEntities.clear();
    for (std::size_t i : m_registry.view<ecs::Player>())
        m_registry.kill(m_registry.entityAt(i));
    m_registry.get<ecs::Match>(*m_match).setPhase(ecs::Match::Phase::Waiting);
}

ecs::Entity GameView::createRemote(const std::string &prefab, float x, float y)
{
    const ecs::Entity entity = m_prefabs.spawn(prefab);
    m_registry.emplace<ecs::Transform>(entity, x, y);
    return entity;
}

std::optional<ecs::Entity> GameView::playerEntity(std::size_t slot) const
{
    const auto &players = m_registry.getComponents<ecs::Player>();
    for (std::size_t i : m_registry.view<ecs::Player>()) {
        if (players.at(i).slot() == slot)
            return m_registry.entityAt(i);
    }
    return std::nullopt;
}

std::uint8_t GameView::parseKind(const std::string &text)
{
    unsigned int value = 0;
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size() || value > MAX_KIND)
        throw ClientError("GameView: invalid entity kind " + text);
    return static_cast<std::uint8_t>(value);
}

float GameView::verticalDirection(float previousY, float nextY)
{
    if (nextY < previousY)
        return 1.f;
    if (nextY > previousY)
        return -1.f;
    return 0.f;
}

void GameView::applyVisual(const ecs::Entity &entity, std::uint8_t visual)
{
    if (visual & network::Protocol::VISUAL_FLASH) {
        m_registry.emplace<ecs::HitFlash>(entity, ecs::HitFlash::DEFAULT_SECONDS);
    }
    const auto step = static_cast<float>(visual & network::Protocol::VISUAL_ANGLE_MASK);
    if (step == 0.f && !m_registry.has<ecs::Rotation>(entity))
        return;
    const float degrees = step * 360.f / network::Protocol::VISUAL_ANGLE_STEPS;
    m_registry.emplace<ecs::Rotation>(entity, degrees);
}

} // namespace engine::client
