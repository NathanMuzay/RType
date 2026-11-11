#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine::ecs {

class UiStore
{
public:
    using Row = std::unordered_map<std::string, std::string>;

    UiStore();

    void setValue(const std::string &key, std::string value);
    std::string value(const std::string &key) const;
    void setList(const std::string &name, std::vector<Row> rows);
    const std::vector<Row> &list(const std::string &name) const;
    std::uint64_t listVersion(const std::string &name) const;

    std::string lookup(const std::string &key, const Row *row) const;
    std::string format(const std::string &text, const Row *row) const;

private:
    std::unordered_map<std::string, std::string> m_values;
    std::unordered_map<std::string, std::vector<Row>> m_lists;
    std::unordered_map<std::string, std::uint64_t> m_versions;
    std::uint64_t m_nextVersion;
    std::vector<Row> m_empty;
};

} // namespace engine::ecs
