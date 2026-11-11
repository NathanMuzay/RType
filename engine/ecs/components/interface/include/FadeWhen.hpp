#pragma once

#include <string>

#include "IComponent.hpp"

namespace engine::ecs {

class FadeWhen : public IComponent
{
public:
    FadeWhen(std::string key, std::string value, float seconds);

    const std::string &key() const;
    const std::string &value() const;
    float opacity() const;
    void fade(bool visible, float dt);

private:
    std::string m_key;
    std::string m_value;
    float m_seconds;
    float m_opacity;
};

} // namespace engine::ecs
