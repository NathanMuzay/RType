#include "Owner.hpp"

namespace engine::ecs {

Owner::Owner(Entity owner) : m_owner(owner)
{
}

Entity Owner::owner() const
{
    return m_owner;
}

} // namespace engine::ecs
