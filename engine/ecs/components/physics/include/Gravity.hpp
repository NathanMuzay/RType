#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class Gravity : public IComponent
{
public:
    explicit Gravity(float force);

    float force() const;

private:
    float m_force;
};

} // namespace engine::ecs
