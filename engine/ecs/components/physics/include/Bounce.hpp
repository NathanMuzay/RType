#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class Bounce : public IComponent
{
public:
    explicit Bounce(float restitution);

    float restitution() const;

private:
    float m_restitution;
};

} // namespace engine::ecs
