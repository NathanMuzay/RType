#include "Invulnerable.hpp"

namespace engine::ecs {

Invulnerable::Invulnerable(float seconds) : m_remaining(seconds)
{
}

float Invulnerable::remaining() const
{
    return m_remaining;
}

void Invulnerable::elapse(float dt)
{
    m_remaining -= dt;
}

bool Invulnerable::expired() const
{
    return m_remaining <= 0.f;
}

} // namespace engine::ecs
