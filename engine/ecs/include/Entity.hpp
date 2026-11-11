#pragma once

#include <cstddef>
#include <cstdint>

namespace engine::ecs {

class EntityManager;

class Entity
{
public:
    std::size_t index() const noexcept;
    std::uint32_t generation() const noexcept;

    bool operator==(const Entity &other) const noexcept = default;

private:
    Entity(std::size_t index, std::uint32_t generation) noexcept;
    friend class EntityManager;

    std::size_t m_index;
    std::uint32_t m_generation;
};

} // namespace engine::ecs
