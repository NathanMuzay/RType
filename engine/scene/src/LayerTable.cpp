#include "LayerTable.hpp"

#include "SceneError.hpp"

namespace engine::scene {

void LayerTable::load(const DataNode &names)
{
    m_bits.clear();
    for (const DataNode &name : names.items()) {
        if (m_bits.size() >= MAX_LAYERS)
            throw SceneError("LayerTable: too many collision layers in " + names.path());
        const std::uint32_t bit = std::uint32_t{1} << m_bits.size();
        if (!m_bits.emplace(name.asString(), bit).second)
            throw SceneError("LayerTable: duplicate layer " + name.asString());
    }
}

std::uint32_t LayerTable::bit(const std::string &name) const
{
    const auto found = m_bits.find(name);
    if (found == m_bits.end())
        throw SceneError("LayerTable: unknown layer " + name);
    return found->second;
}

std::uint32_t LayerTable::mask(const DataNode &names) const
{
    std::uint32_t mask = 0;
    for (const DataNode &name : names.items())
        mask |= bit(name.asString());
    return mask;
}

} // namespace engine::scene
