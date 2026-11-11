#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class Transform : public IComponent
{
public:
    explicit Transform(float x = 0.f, float y = 0.f);

    float x() const;
    float y() const;
    void setPosition(float x, float y);
    void translate(float dx, float dy);

private:
    float m_x;
    float m_y;
};

} // namespace engine::ecs
