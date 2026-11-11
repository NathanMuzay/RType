#pragma once

#include <string>

#include "IComponent.hpp"
#include "IntRect.hpp"

namespace engine::ecs {

class Sprite : public IComponent
{
public:
    Sprite(std::string texture, IntRect source, float scale);

    const std::string &texture() const;
    IntRect source() const;
    void setSource(IntRect source);
    float scale() const;

private:
    std::string m_texture;
    IntRect m_source;
    float m_scale;
};

} // namespace engine::ecs
