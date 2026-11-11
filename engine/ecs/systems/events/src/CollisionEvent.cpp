#include "CollisionEvent.hpp"

namespace engine::ecs {

CollisionEvent::CollisionEvent(Entity first, Entity second) : m_first(first), m_second(second)
{
}

Entity CollisionEvent::first() const
{
    return m_first;
}

Entity CollisionEvent::second() const
{
    return m_second;
}

} // namespace engine::ecs
