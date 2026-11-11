#pragma once

#include <optional>

#include "CollisionEvent.hpp"
#include "DeathEvent.hpp"
#include "Entity.hpp"
#include "EventQueue.hpp"
#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class DamageSystem : public ISystem
{
public:
    DamageSystem(Registry &registry, EventQueue<CollisionEvent> &collisions, EventQueue<DeathEvent> &deaths);

    void update(float dt) override;

private:
    void apply(const Entity &source, const Entity &target);
    bool canInteract(const Entity &source, const Entity &target) const;
    void markDestroyed(const Entity &entity, std::optional<Entity> killer);
    Entity creditOf(const Entity &source) const;

    Registry &m_registry;
    EventQueue<CollisionEvent> &m_collisions;
    EventQueue<DeathEvent> &m_deaths;
};

} // namespace engine::ecs
