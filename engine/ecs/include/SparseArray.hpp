#pragma once

#include <cstddef>
#include <optional>
#include <type_traits>
#include <vector>

#include "IComponent.hpp"
#include "ISparseArray.hpp"

namespace engine::ecs {

template <class Component>
class SparseArray : public ISparseArray
{
    static_assert(std::is_base_of_v<IComponent, Component>, "SparseArray: a component must derive from IComponent");

public:
    template <class... Args>
    Component &emplaceAt(std::size_t index, Args &&...args);

    void erase(std::size_t index) override;
    bool has(std::size_t index) const override;

    Component &at(std::size_t index);
    const Component &at(std::size_t index) const;
    std::size_t size() const;

private:
    std::vector<std::optional<Component>> m_data;
};

} // namespace engine::ecs

#include "SparseArray.tpp"
