#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class Velocity : public IComponent
{
public:
    explicit Velocity(float x = 0.f, float y = 0.f);

    float x() const;
    float y() const;
    void set(float x, float y);
    void setX(float x);
    void setY(float y);

private:
    float m_x;
    float m_y;
};

} // namespace engine::ecs
