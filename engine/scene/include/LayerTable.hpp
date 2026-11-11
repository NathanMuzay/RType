#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "DataNode.hpp"

namespace engine::scene {

class LayerTable
{
public:
    static constexpr std::size_t MAX_LAYERS = 32;

    void load(const DataNode &names);
    std::uint32_t bit(const std::string &name) const;
    std::uint32_t mask(const DataNode &names) const;

private:
    std::unordered_map<std::string, std::uint32_t> m_bits;
};

} // namespace engine::scene
