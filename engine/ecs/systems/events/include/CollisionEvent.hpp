#pragma once

#include "Entity.hpp"

namespace engine::ecs {

class CollisionEvent
{
public:
    CollisionEvent(Entity first, Entity second);

    Entity first() const;
    Entity second() const;

private:
    Entity m_first;
    Entity m_second;
};

} // namespace engine::ecs
