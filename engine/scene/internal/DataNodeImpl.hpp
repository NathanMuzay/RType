#pragma once

#include <memory>
#include <string>

#include <nlohmann/json.hpp>

#include "DataNode.hpp"

namespace engine::scene {

class DataNode::Impl
{
public:
    Impl(std::shared_ptr<const nlohmann::json> root, const nlohmann::json *node, std::string path);

    const nlohmann::json &node() const;
    const std::shared_ptr<const nlohmann::json> &root() const;
    const std::string &path() const;
    std::string pathOf(const std::string &key) const;
    const nlohmann::json &member(const std::string &key) const;

    static float toFloat(const nlohmann::json &value, const std::string &path);
    static int toInt(const nlohmann::json &value, const std::string &path);
    static bool toBool(const nlohmann::json &value, const std::string &path);
    static std::string toString(const nlohmann::json &value, const std::string &path);

private:
    std::shared_ptr<const nlohmann::json> m_root;
    const nlohmann::json *m_node;
    std::string m_path;
};

} // namespace engine::scene
