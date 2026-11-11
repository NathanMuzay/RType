#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class Lifetime : public IComponent
{
public:
    explicit Lifetime(float seconds);

    float total() const;
    float remaining() const;
    void elapse(float dt);

private:
    float m_total;
    float m_remaining;
};

} // namespace engine::ecs
