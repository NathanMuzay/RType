#pragma once

#include <utility>

#include "EcsError.hpp"

namespace engine::ecs {

template <class Component>
SparseArray<Component> &Registry::registerComponent()
{
    const std::type_index type(typeid(Component));
    const auto found = m_bits.find(type);
    if (found != m_bits.end())
        return arrayAt<Component>(found->second);
    if (m_arrays.size() >= MAX_COMPONENT_TYPES)
        throw EcsError("Registry: too many component types");
    m_bits.emplace(type, m_arrays.size());
    m_arrays.push_back(std::make_unique<SparseArray<Component>>());
    return arrayAt<Component>(m_arrays.size() - 1);
}

template <class Component>
SparseArray<Component> &Registry::getComponents()
{
    return arrayAt<Component>(bitOf<Component>());
}

template <class Component>
const SparseArray<Component> &Registry::getComponents() const
{
    return arrayAt<Component>(bitOf<Component>());
}

template <class Component, class... Args>
Component &Registry::emplace(const Entity &entity, Args &&...args)
{
    const std::size_t bit = bitOf<Component>();
    if (!m_entities.isAlive(entity))
        throw EcsError("Registry: the entity is dead");
    Component &component = arrayAt<Component>(bit).emplaceAt(entity.index(), std::forward<Args>(args)...);
    m_entities.addComponents(entity, maskOf(bit));
    return component;
}

template <class Component>
void Registry::remove(const Entity &entity)
{
    const std::size_t bit = bitOf<Component>();
    if (!m_entities.isAlive(entity))
        return;
    m_arrays[bit]->erase(entity.index());
    m_entities.removeComponents(entity, maskOf(bit));
}

template <class Component>
bool Registry::has(const Entity &entity) const
{
    return m_entities.hasComponents(entity, maskOf(bitOf<Component>()));
}

template <class Component>
Component &Registry::get(const Entity &entity)
{
    if (!has<Component>(entity))
        throw EcsError("Registry: the entity does not have this component");
    return getComponents<Component>().at(entity.index());
}

template <class Component>
const Component &Registry::get(const Entity &entity) const
{
    if (!has<Component>(entity))
        throw EcsError("Registry: the entity does not have this component");
    return getComponents<Component>().at(entity.index());
}

template <class... Components>
std::vector<std::size_t> Registry::view() const
{
    return m_entities.with((ComponentMask{0} | ... | maskOf(bitOf<Components>())));
}

template <class Component>
std::size_t Registry::bitOf() const
{
    const auto found = m_bits.find(std::type_index(typeid(Component)));
    if (found == m_bits.end())
        throw EcsError("Registry: component type not registered");
    return found->second;
}

template <class Component>
SparseArray<Component> &Registry::arrayAt(std::size_t bit)
{
    return static_cast<SparseArray<Component> &>(*m_arrays[bit]);
}

template <class Component>
const SparseArray<Component> &Registry::arrayAt(std::size_t bit) const
{
    return static_cast<const SparseArray<Component> &>(*m_arrays[bit]);
}

} // namespace engine::ecs
