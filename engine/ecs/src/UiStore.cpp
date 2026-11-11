#include "UiStore.hpp"

#include <utility>

namespace engine::ecs {

UiStore::UiStore() : m_values(), m_lists(), m_versions(), m_nextVersion(1), m_empty()
{
}

void UiStore::setValue(const std::string &key, std::string value)
{
    m_values.insert_or_assign(key, std::move(value));
}

std::string UiStore::value(const std::string &key) const
{
    const auto found = m_values.find(key);
    return found == m_values.end() ? std::string() : found->second;
}

void UiStore::setList(const std::string &name, std::vector<Row> rows)
{
    m_lists.insert_or_assign(name, std::move(rows));
    m_versions.insert_or_assign(name, m_nextVersion++);
}

const std::vector<UiStore::Row> &UiStore::list(const std::string &name) const
{
    const auto found = m_lists.find(name);
    return found == m_lists.end() ? m_empty : found->second;
}

std::uint64_t UiStore::listVersion(const std::string &name) const
{
    const auto found = m_versions.find(name);
    return found == m_versions.end() ? 0 : found->second;
}

std::string UiStore::lookup(const std::string &key, const Row *row) const
{
    if (row) {
        const auto found = row->find(key);
        if (found != row->end())
            return found->second;
    }
    return value(key);
}

std::string UiStore::format(const std::string &text, const Row *row) const
{
    std::string result;
    std::size_t position = 0;
    while (position < text.size()) {
        const std::size_t open = text.find('{', position);
        const std::size_t close = open == std::string::npos ? std::string::npos : text.find('}', open);
        if (close == std::string::npos) {
            result.append(text, position, std::string::npos);
            break;
        }
        result.append(text, position, open - position);
        result += lookup(text.substr(open + 1, close - open - 1), row);
        position = close + 1;
    }
    return result;
}

} // namespace engine::ecs
