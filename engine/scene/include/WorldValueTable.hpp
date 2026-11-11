#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "DataNode.hpp"

namespace engine::scene {

class WorldValueTable
{
public:
    static constexpr std::size_t MAX_VALUES = 32;

    WorldValueTable();

    void load(const DataNode &list);
    std::size_t size() const;
    const std::string &name(std::size_t index) const;
    std::optional<std::size_t> index(const std::string &name) const;
    std::string label(std::size_t index, int value) const;
    std::optional<int> valueOf(std::size_t index, const std::string &label) const;

private:
    std::vector<std::string> m_names;
    std::vector<std::vector<std::string>> m_labels;
};

} // namespace engine::scene
