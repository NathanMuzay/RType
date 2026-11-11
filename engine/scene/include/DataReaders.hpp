#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Color.hpp"
#include "DataNode.hpp"
#include "IntRect.hpp"

namespace engine::scene {

class DataReaders
{
public:
    static constexpr std::size_t RECT_VALUES = 4;
    static constexpr std::size_t COLOR_VALUES = 4;

    static IntRect readRect(const DataNode &node, const std::string &key);
    static Color readColor(const DataNode &values);
    static Color readColor(const DataNode &node, const std::string &key);
    static std::vector<Color> readColors(const DataNode &node, const std::string &key);
};

} // namespace engine::scene
