#include "Script.hpp"

#include <utility>

namespace engine::ecs {

Script::Script(std::string file, Numbers numbers, Texts texts)
    : m_file(std::move(file)), m_numbers(std::move(numbers)), m_texts(std::move(texts))
{
}

const std::string &Script::file() const
{
    return m_file;
}

float Script::number(const std::string &name, float fallback) const
{
    const auto found = m_numbers.find(name);
    return found == m_numbers.end() ? fallback : found->second;
}

std::string Script::text(const std::string &name, const std::string &fallback) const
{
    const auto found = m_texts.find(name);
    return found == m_texts.end() ? fallback : found->second;
}

} // namespace engine::ecs
