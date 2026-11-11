#include "DeathEvent.hpp"

namespace engine::ecs {

DeathEvent::DeathEvent(Entity victim, std::optional<Entity> killer) : m_victim(victim), m_killer(killer)
{
}

Entity DeathEvent::victim() const
{
    return m_victim;
}

std::optional<Entity> DeathEvent::killer() const
{
    return m_killer;
}

} // namespace engine::ecs
