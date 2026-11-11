#pragma once

#include "CollisionEvent.hpp"
#include "EventQueue.hpp"
#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class CollisionSystem : public ISystem
{
public:
    CollisionSystem(Registry &registry, EventQueue<CollisionEvent> &collisions);

    void update(float dt) override;

private:
    Registry &m_registry;
    EventQueue<CollisionEvent> &m_collisions;
};

} // namespace engine::ecs
