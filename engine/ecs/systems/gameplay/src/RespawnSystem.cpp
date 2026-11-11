#include "RespawnSystem.hpp"

#include <string>

#include "Destroyed.hpp"
#include "Invulnerable.hpp"
#include "Lives.hpp"
#include "MatchSystem.hpp"
#include "Owner.hpp"
#include "Respawn.hpp"
#include "Transform.hpp"

namespace engine::ecs {

RespawnSystem::RespawnSystem(Registry &registry, IEntityFactory &factory) : m_registry(registry), m_factory(factory)
{
}

void RespawnSystem::update(float dt)
{
    if (MatchSystem::phaseOf(m_registry) != Match::Phase::Playing)
        return;
    for (std::size_t i : m_registry.view<Player, Respawn>()) {
        const Entity owner = m_registry.entityAt(i);
        if (avatarAlive(m_registry.get<Player>(owner)))
            continue;
        Respawn &respawn = m_registry.get<Respawn>(owner);
        if (!respawn.waiting())
            respawn.schedule();
        if (m_registry.has<Lives>(owner) && m_registry.get<Lives>(owner).count() <= 0)
            continue;
        respawn.elapse(dt);
        if (respawn.timer() <= 0.f)
            spawnAvatar(owner);
    }
}

bool RespawnSystem::avatarAlive(const Player &player) const
{
    const auto avatar = player.avatar();
    return avatar && Destroyed::isActive(m_registry, *avatar);
}

void RespawnSystem::spawnAvatar(const Entity &owner)
{
    const Respawn &settings = m_registry.get<Respawn>(owner);
    const float slot = static_cast<float>(m_registry.get<Player>(owner).slot());
    const std::string prefab = settings.prefab();
    const float x = settings.x() + slot * settings.spacingX();
    const float y = settings.y() + slot * settings.spacingY();
    const bool invulnerable = !settings.firstSpawn();
    const float invulnerability = settings.invulnerability();

    const Entity avatar = m_factory.spawn(prefab);
    m_registry.emplace<Transform>(avatar, x, y);
    m_registry.emplace<Owner>(avatar, owner);
    if (invulnerable)
        m_registry.emplace<Invulnerable>(avatar, invulnerability);
    m_registry.get<Respawn>(owner).spawned();
    m_registry.get<Player>(owner).setAvatar(avatar);
}

} // namespace engine::ecs
