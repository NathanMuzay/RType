#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class Invulnerable : public IComponent
{
public:
    explicit Invulnerable(float seconds);

    float remaining() const;
    void elapse(float dt);
    bool expired() const;

private:
    float m_remaining;
};

} // namespace engine::ecs
