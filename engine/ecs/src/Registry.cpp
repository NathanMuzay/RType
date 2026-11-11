#include "Registry.hpp"

namespace engine::ecs {

Entity Registry::spawn()
{
    return m_entities.create();
}

void Registry::kill(const Entity &entity)
{
    if (!m_entities.isAlive(entity))
        return;
    for (auto &array : m_arrays)
        array->erase(entity.index());
    m_entities.destroy(entity);
}

bool Registry::isAlive(const Entity &entity) const
{
    return m_entities.isAlive(entity);
}

Entity Registry::entityAt(std::size_t index) const
{
    return m_entities.entityAt(index);
}

ComponentMask Registry::maskOf(std::size_t bit)
{
    return ComponentMask{1} << bit;
}

} // namespace engine::ecs
