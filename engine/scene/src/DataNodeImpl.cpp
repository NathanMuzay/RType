#include "DataNodeImpl.hpp"

#include <utility>

#include "SceneError.hpp"

namespace engine::scene {

DataNode::Impl::Impl(std::shared_ptr<const nlohmann::json> root, const nlohmann::json *node, std::string path)
    : m_root(std::move(root)), m_node(node), m_path(std::move(path))
{
}

const nlohmann::json &DataNode::Impl::node() const
{
    return *m_node;
}

const std::shared_ptr<const nlohmann::json> &DataNode::Impl::root() const
{
    return m_root;
}

const std::string &DataNode::Impl::path() const
{
    return m_path;
}

std::string DataNode::Impl::pathOf(const std::string &key) const
{
    return m_path + "." + key;
}

const nlohmann::json &DataNode::Impl::member(const std::string &key) const
{
    if (!m_node->is_object() || !m_node->contains(key))
        throw SceneError("DataNode: missing key " + pathOf(key));
    return m_node->at(key);
}

float DataNode::Impl::toFloat(const nlohmann::json &value, const std::string &path)
{
    if (!value.is_number())
        throw SceneError("DataNode: " + path + " must be a number");
    return value.get<float>();
}

int DataNode::Impl::toInt(const nlohmann::json &value, const std::string &path)
{
    if (!value.is_number_integer())
        throw SceneError("DataNode: " + path + " must be an integer");
    return value.get<int>();
}

bool DataNode::Impl::toBool(const nlohmann::json &value, const std::string &path)
{
    if (!value.is_boolean())
        throw SceneError("DataNode: " + path + " must be a boolean");
    return value.get<bool>();
}

std::string DataNode::Impl::toString(const nlohmann::json &value, const std::string &path)
{
    if (!value.is_string())
        throw SceneError("DataNode: " + path + " must be a string");
    return value.get<std::string>();
}

} // namespace engine::scene
