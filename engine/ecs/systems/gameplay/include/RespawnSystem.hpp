#pragma once

#include "Entity.hpp"
#include "IEntityFactory.hpp"
#include "ISystem.hpp"
#include "Player.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class RespawnSystem : public ISystem
{
public:
    RespawnSystem(Registry &registry, IEntityFactory &factory);

    void update(float dt) override;

private:
    bool avatarAlive(const Player &player) const;
    void spawnAvatar(const Entity &owner);

    Registry &m_registry;
    IEntityFactory &m_factory;
};

} // namespace engine::ecs
