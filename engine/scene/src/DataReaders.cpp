#include "DataReaders.hpp"

#include <cstdint>

#include "SceneError.hpp"

namespace engine::scene {

IntRect DataReaders::readRect(const DataNode &node, const std::string &key)
{
    const std::vector<DataNode> values = node.child(key).items();
    if (values.size() != RECT_VALUES)
        throw SceneError("DataReaders: " + node.path() + "." + key + " must be [x, y, width, height]");
    return {values[0].asInt(), values[1].asInt(), values[2].asInt(), values[3].asInt()};
}

Color DataReaders::readColor(const DataNode &values)
{
    const std::vector<DataNode> channels = values.items();
    if (channels.size() != COLOR_VALUES)
        throw SceneError("DataReaders: " + values.path() + " must be [r, g, b, a]");
    return {static_cast<std::uint8_t>(channels[0].asInt()), static_cast<std::uint8_t>(channels[1].asInt()),
        static_cast<std::uint8_t>(channels[2].asInt()), static_cast<std::uint8_t>(channels[3].asInt())};
}

Color DataReaders::readColor(const DataNode &node, const std::string &key)
{
    return readColor(node.child(key));
}

std::vector<Color> DataReaders::readColors(const DataNode &node, const std::string &key)
{
    std::vector<Color> colors;
    for (const DataNode &color : node.child(key).items())
        colors.push_back(readColor(color));
    return colors;
}

} // namespace engine::scene
