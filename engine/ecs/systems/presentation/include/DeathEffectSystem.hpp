#pragma once

#include "DeathEvent.hpp"
#include "Entity.hpp"
#include "EventQueue.hpp"
#include "IEntityFactory.hpp"
#include "ISystem.hpp"
#include "Registry.hpp"
#include "Vector2.hpp"

namespace engine::ecs {

class DeathEffectSystem : public ISystem
{
public:
    DeathEffectSystem(Registry &registry, IEntityFactory &factory, const EventQueue<DeathEvent> &deaths);

    void update(float dt) override;

private:
    Vector2 halfSizeOf(const Entity &entity) const;

    Registry &m_registry;
    IEntityFactory &m_factory;
    const EventQueue<DeathEvent> &m_deaths;
};

} // namespace engine::ecs
