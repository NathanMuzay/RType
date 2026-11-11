#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace engine::scene {

class DataNode
{
public:
    static DataNode fromFile(const std::filesystem::path &path);

    bool has(const std::string &key) const;
    bool isString() const;
    DataNode child(const std::string &key) const;
    std::vector<std::string> keys() const;
    std::vector<DataNode> items() const;

    float getFloat(const std::string &key) const;
    float getFloat(const std::string &key, float fallback) const;
    int getInt(const std::string &key) const;
    int getInt(const std::string &key, int fallback) const;
    bool getBool(const std::string &key, bool fallback) const;
    std::string getString(const std::string &key) const;

    float asFloat() const;
    int asInt() const;
    std::string asString() const;
    const std::string &path() const;

private:
    class Impl;

    explicit DataNode(std::shared_ptr<const Impl> impl);

    std::shared_ptr<const Impl> m_impl;
};

} // namespace engine::scene
