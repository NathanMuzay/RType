#pragma once

#include <cstddef>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "Entity.hpp"
#include "EntityManager.hpp"
#include "ISparseArray.hpp"
#include "SparseArray.hpp"

namespace engine::ecs {

class Registry
{
public:
    static constexpr std::size_t MAX_COMPONENT_TYPES = 64;

    template <class Component>
    SparseArray<Component> &registerComponent();
    template <class Component>
    SparseArray<Component> &getComponents();
    template <class Component>
    const SparseArray<Component> &getComponents() const;

    Entity spawn();
    void kill(const Entity &entity);
    bool isAlive(const Entity &entity) const;
    Entity entityAt(std::size_t index) const;

    template <class Component, class... Args>
    Component &emplace(const Entity &entity, Args &&...args);
    template <class Component>
    void remove(const Entity &entity);
    template <class Component>
    bool has(const Entity &entity) const;
    template <class Component>
    Component &get(const Entity &entity);
    template <class Component>
    const Component &get(const Entity &entity) const;

    template <class... Components>
    std::vector<std::size_t> view() const;

private:
    template <class Component>
    std::size_t bitOf() const;
    template <class Component>
    SparseArray<Component> &arrayAt(std::size_t bit);
    template <class Component>
    const SparseArray<Component> &arrayAt(std::size_t bit) const;

    static ComponentMask maskOf(std::size_t bit);

    EntityManager m_entities;
    std::unordered_map<std::type_index, std::size_t> m_bits;
    std::vector<std::unique_ptr<ISparseArray>> m_arrays;
};

} // namespace engine::ecs

#include "Registry.tpp"
