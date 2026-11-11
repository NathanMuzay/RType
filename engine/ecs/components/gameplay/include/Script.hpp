#pragma once

#include <string>
#include <unordered_map>

#include "IComponent.hpp"

namespace engine::ecs {

class Script : public IComponent
{
public:
    using Numbers = std::unordered_map<std::string, float>;
    using Texts = std::unordered_map<std::string, std::string>;

    Script(std::string file, Numbers numbers, Texts texts);

    const std::string &file() const;
    float number(const std::string &name, float fallback) const;
    std::string text(const std::string &name, const std::string &fallback) const;

private:
    std::string m_file;
    Numbers m_numbers;
    Texts m_texts;
};

} // namespace engine::ecs
