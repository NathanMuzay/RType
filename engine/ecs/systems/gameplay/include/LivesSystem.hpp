#pragma once

#include "DeathEvent.hpp"
#include "EventQueue.hpp"
#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class LivesSystem : public ISystem
{
public:
    LivesSystem(Registry &registry, const EventQueue<DeathEvent> &deaths);

    void update(float dt) override;

private:
    Registry &m_registry;
    const EventQueue<DeathEvent> &m_deaths;
};

} // namespace engine::ecs
