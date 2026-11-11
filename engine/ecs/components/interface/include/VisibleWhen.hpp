#pragma once

#include <string>
#include <utility>
#include <vector>

#include "IComponent.hpp"

namespace engine::ecs {

class VisibleWhen : public IComponent
{
public:
    using Condition = std::pair<std::string, std::string>;

    VisibleWhen(std::string key, std::string value, std::vector<Condition> also = {}, std::string unlessKey = {},
        std::string unlessValue = {});

    const std::string &key() const;
    const std::string &value() const;
    const std::vector<Condition> &also() const;
    const std::string &unlessKey() const;
    const std::string &unlessValue() const;

private:
    std::string m_key;
    std::string m_value;
    std::vector<Condition> m_also;
    std::string m_unlessKey;
    std::string m_unlessValue;
};

} // namespace engine::ecs
