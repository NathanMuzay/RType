#include "CollisionPair.hpp"

namespace engine::physics {

CollisionPair::CollisionPair(std::size_t first, std::size_t second) : m_first(first), m_second(second)
{
}

std::size_t CollisionPair::first() const
{
    return m_first;
}

std::size_t CollisionPair::second() const
{
    return m_second;
}

} // namespace engine::physics
