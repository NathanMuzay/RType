#pragma once

#include <utility>

#include "EcsError.hpp"

namespace engine::ecs {

template <class Component>
template <class... Args>
Component &SparseArray<Component>::emplaceAt(std::size_t index, Args &&...args)
{
    if (index >= m_data.size())
        m_data.resize(index + 1);
    m_data[index].emplace(std::forward<Args>(args)...);
    return *m_data[index];
}

template <class Component>
void SparseArray<Component>::erase(std::size_t index)
{
    if (index < m_data.size())
        m_data[index].reset();
}

template <class Component>
bool SparseArray<Component>::has(std::size_t index) const
{
    return index < m_data.size() && m_data[index].has_value();
}

template <class Component>
Component &SparseArray<Component>::at(std::size_t index)
{
    if (!has(index))
        throw EcsError("SparseArray: no component at this index");
    return *m_data[index];
}

template <class Component>
const Component &SparseArray<Component>::at(std::size_t index) const
{
    if (!has(index))
        throw EcsError("SparseArray: no component at this index");
    return *m_data[index];
}

template <class Component>
std::size_t SparseArray<Component>::size() const
{
    return m_data.size();
}

} // namespace engine::ecs
