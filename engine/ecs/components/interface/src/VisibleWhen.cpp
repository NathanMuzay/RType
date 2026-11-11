#include "VisibleWhen.hpp"

#include <utility>

namespace engine::ecs {

VisibleWhen::VisibleWhen(
    std::string key, std::string value, std::vector<Condition> also, std::string unlessKey, std::string unlessValue)
    : m_key(std::move(key)), m_value(std::move(value)), m_also(std::move(also)), m_unlessKey(std::move(unlessKey)),
      m_unlessValue(std::move(unlessValue))
{
}

const std::string &VisibleWhen::key() const
{
    return m_key;
}

const std::string &VisibleWhen::value() const
{
    return m_value;
}

const std::vector<VisibleWhen::Condition> &VisibleWhen::also() const
{
    return m_also;
}

const std::string &VisibleWhen::unlessKey() const
{
    return m_unlessKey;
}

const std::string &VisibleWhen::unlessValue() const
{
    return m_unlessValue;
}

} // namespace engine::ecs
