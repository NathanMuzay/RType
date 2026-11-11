#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "Entity.hpp"

namespace engine::ecs {

using ComponentMask = std::uint64_t;

class EntityManager
{
public:
    explicit EntityManager(std::size_t capacity = 0);

    Entity create();
    void destroy(const Entity &entity);
    bool isAlive(const Entity &entity) const;
    Entity entityAt(std::size_t index) const;

    void addComponents(const Entity &entity, ComponentMask components);
    void removeComponents(const Entity &entity, ComponentMask components);
    bool hasComponents(const Entity &entity, ComponentMask components) const;

    std::vector<std::size_t> with(ComponentMask components) const;

private:
    void checkAlive(const Entity &entity) const;
    void grow();

    std::vector<ComponentMask> m_masks;
    std::vector<std::uint32_t> m_generations;
    std::vector<bool> m_alive;
    std::vector<std::size_t> m_freeIndexes;
    std::size_t m_capacity;
};

} // namespace engine::ecs
