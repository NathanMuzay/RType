#pragma once

#include <optional>

#include "Entity.hpp"

namespace engine::ecs {

class DeathEvent
{
public:
    DeathEvent(Entity victim, std::optional<Entity> killer);

    Entity victim() const;
    std::optional<Entity> killer() const;

private:
    Entity m_victim;
    std::optional<Entity> m_killer;
};

} // namespace engine::ecs
