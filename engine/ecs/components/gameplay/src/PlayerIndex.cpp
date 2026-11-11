#include "PlayerIndex.hpp"

namespace engine::ecs {

PlayerIndex::PlayerIndex(std::size_t slot) : m_slot(slot)
{
}

std::size_t PlayerIndex::slot() const
{
    return m_slot;
}

} // namespace engine::ecs
