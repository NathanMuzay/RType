#include "DataNode.hpp"

#include <fstream>
#include <utility>

#include "DataNodeImpl.hpp"
#include "SceneError.hpp"

namespace engine::scene {

DataNode::DataNode(std::shared_ptr<const Impl> impl) : m_impl(std::move(impl))
{
}

DataNode DataNode::fromFile(const std::filesystem::path &path)
{
    std::ifstream file(path);
    if (!file)
        throw SceneError("DataNode: cannot open " + path.string());
    try {
        auto root = std::make_shared<const nlohmann::json>(nlohmann::json::parse(file));
        const nlohmann::json *node = root.get();
        return DataNode(std::make_shared<const Impl>(root, node, path.filename().string()));
    } catch (const nlohmann::json::exception &error) {
        throw SceneError("DataNode: invalid JSON in " + path.string() + ": " + error.what());
    }
}

bool DataNode::has(const std::string &key) const
{
    return m_impl->node().is_object() && m_impl->node().contains(key);
}

bool DataNode::isString() const
{
    return m_impl->node().is_string();
}

DataNode DataNode::child(const std::string &key) const
{
    return DataNode(std::make_shared<const Impl>(m_impl->root(), &m_impl->member(key), m_impl->pathOf(key)));
}

std::vector<std::string> DataNode::keys() const
{
    if (!m_impl->node().is_object())
        throw SceneError("DataNode: " + m_impl->path() + " must be an object");
    std::vector<std::string> keys;
    for (const auto &item : m_impl->node().items())
        keys.push_back(item.key());
    return keys;
}

std::vector<DataNode> DataNode::items() const
{
    if (!m_impl->node().is_array())
        throw SceneError("DataNode: " + m_impl->path() + " must be an array");
    std::vector<DataNode> items;
    std::size_t index = 0;
    for (const nlohmann::json &value : m_impl->node()) {
        const std::string path = m_impl->path() + "[" + std::to_string(index++) + "]";
        items.push_back(DataNode(std::make_shared<const Impl>(m_impl->root(), &value, path)));
    }
    return items;
}

float DataNode::getFloat(const std::string &key) const
{
    return Impl::toFloat(m_impl->member(key), m_impl->pathOf(key));
}

float DataNode::getFloat(const std::string &key, float fallback) const
{
    return has(key) ? getFloat(key) : fallback;
}

int DataNode::getInt(const std::string &key) const
{
    return Impl::toInt(m_impl->member(key), m_impl->pathOf(key));
}

int DataNode::getInt(const std::string &key, int fallback) const
{
    return has(key) ? getInt(key) : fallback;
}

bool DataNode::getBool(const std::string &key, bool fallback) const
{
    return has(key) ? Impl::toBool(m_impl->member(key), m_impl->pathOf(key)) : fallback;
}

std::string DataNode::getString(const std::string &key) const
{
    return Impl::toString(m_impl->member(key), m_impl->pathOf(key));
}

float DataNode::asFloat() const
{
    return Impl::toFloat(m_impl->node(), m_impl->path());
}

int DataNode::asInt() const
{
    return Impl::toInt(m_impl->node(), m_impl->path());
}

std::string DataNode::asString() const
{
    return Impl::toString(m_impl->node(), m_impl->path());
}

const std::string &DataNode::path() const
{
    return m_impl->path();
}

} // namespace engine::scene
