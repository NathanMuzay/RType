#include "Entity.hpp"

namespace engine::ecs {

Entity::Entity(std::size_t index, std::uint32_t generation) noexcept : m_index(index), m_generation(generation)
{
}

std::size_t Entity::index() const noexcept
{
    return m_index;
}

std::uint32_t Entity::generation() const noexcept
{
    return m_generation;
}

} // namespace engine::ecs
