#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class Parallax : public IComponent
{
public:
    Parallax(float speed, float wrapWidth);

    float speed() const;
    float wrapWidth() const;

private:
    float m_speed;
    float m_wrapWidth;
};

} // namespace engine::ecs
