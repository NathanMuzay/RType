#include "WorldValueTable.hpp"

#include <algorithm>

#include "SceneError.hpp"

namespace engine::scene {

WorldValueTable::WorldValueTable() : m_names(), m_labels()
{
}

void WorldValueTable::load(const DataNode &list)
{
    m_names.clear();
    m_labels.clear();
    for (const DataNode &entry : list.items()) {
        const std::string name = entry.isString() ? entry.asString() : entry.getString("name");
        if (index(name))
            throw SceneError("WorldValueTable: duplicate world value " + name);
        std::vector<std::string> labels;
        if (!entry.isString() && entry.has("labels")) {
            for (const DataNode &label : entry.child("labels").items())
                labels.push_back(label.asString());
        }
        m_names.push_back(name);
        m_labels.push_back(std::move(labels));
    }
    if (m_names.size() > MAX_VALUES)
        throw SceneError("WorldValueTable: too many world values");
}

std::size_t WorldValueTable::size() const
{
    return m_names.size();
}

const std::string &WorldValueTable::name(std::size_t index) const
{
    if (index >= m_names.size())
        throw SceneError("WorldValueTable: no world value at index " + std::to_string(index));
    return m_names[index];
}

std::optional<std::size_t> WorldValueTable::index(const std::string &name) const
{
    const auto found = std::find(m_names.begin(), m_names.end(), name);
    if (found == m_names.end())
        return std::nullopt;
    return static_cast<std::size_t>(found - m_names.begin());
}

std::string WorldValueTable::label(std::size_t index, int value) const
{
    if (index < m_labels.size() && value >= 0 && static_cast<std::size_t>(value) < m_labels[index].size())
        return m_labels[index][static_cast<std::size_t>(value)];
    return std::to_string(value);
}

std::optional<int> WorldValueTable::valueOf(std::size_t index, const std::string &label) const
{
    if (index >= m_labels.size())
        return std::nullopt;
    const auto &labels = m_labels[index];
    const auto found = std::find(labels.begin(), labels.end(), label);
    if (found == labels.end())
        return std::nullopt;
    return static_cast<int>(found - labels.begin());
}

} // namespace engine::scene
