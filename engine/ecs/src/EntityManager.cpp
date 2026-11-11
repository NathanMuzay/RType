#include "EntityManager.hpp"

#include "EcsError.hpp"

namespace engine::ecs {

EntityManager::EntityManager(std::size_t capacity)
    : m_masks(capacity, 0), m_generations(capacity, 0), m_alive(capacity, false), m_capacity(capacity)
{
    for (std::size_t index = capacity; index > 0; --index)
        m_freeIndexes.push_back(index - 1);
}

Entity EntityManager::create()
{
    if (m_freeIndexes.empty())
        grow();
    std::size_t index = m_freeIndexes.back();
    m_freeIndexes.pop_back();
    m_alive[index] = true;
    m_masks[index] = 0;
    return Entity(index, m_generations[index]);
}

void EntityManager::destroy(const Entity &entity)
{
    if (!isAlive(entity))
        return;
    std::size_t index = entity.index();
    m_alive[index] = false;
    m_masks[index] = 0;
    ++m_generations[index];
    m_freeIndexes.push_back(index);
}

bool EntityManager::isAlive(const Entity &entity) const
{
    std::size_t index = entity.index();
    return index < m_alive.size() && m_alive[index] && m_generations[index] == entity.generation();
}

Entity EntityManager::entityAt(std::size_t index) const
{
    if (index >= m_alive.size() || !m_alive[index])
        throw EcsError("EntityManager: no living entity at this index");
    return Entity(index, m_generations[index]);
}

void EntityManager::addComponents(const Entity &entity, ComponentMask components)
{
    checkAlive(entity);
    m_masks[entity.index()] |= components;
}

void EntityManager::removeComponents(const Entity &entity, ComponentMask components)
{
    if (isAlive(entity))
        m_masks[entity.index()] &= ~components;
}

bool EntityManager::hasComponents(const Entity &entity, ComponentMask components) const
{
    return isAlive(entity) && (m_masks[entity.index()] & components) == components;
}

std::vector<std::size_t> EntityManager::with(ComponentMask components) const
{
    std::vector<std::size_t> indexes;
    for (std::size_t index = 0; index < m_masks.size(); ++index) {
        if (m_alive[index] && (m_masks[index] & components) == components)
            indexes.push_back(index);
    }
    return indexes;
}

void EntityManager::checkAlive(const Entity &entity) const
{
    if (!isAlive(entity))
        throw EcsError("EntityManager: the entity is dead");
}

void EntityManager::grow()
{
    if (m_capacity != 0)
        throw EcsError("EntityManager: no free entity left");
    m_masks.push_back(0);
    m_generations.push_back(0);
    m_alive.push_back(false);
    m_freeIndexes.push_back(m_masks.size() - 1);
}

} // namespace engine::ecs
