#pragma once

#include <string>

#include "IComponent.hpp"

namespace engine::ecs {

class ProgressBar : public IComponent
{
public:
    ProgressBar(std::string key, float width, float maximum, bool vertical);

    const std::string &key() const;
    float width() const;
    float maximum() const;
    bool vertical() const;

private:
    std::string m_key;
    float m_width;
    float m_maximum;
    bool m_vertical;
};

} // namespace engine::ecs
