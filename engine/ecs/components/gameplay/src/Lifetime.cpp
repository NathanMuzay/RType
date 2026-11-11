#include "Lifetime.hpp"

namespace engine::ecs {

Lifetime::Lifetime(float seconds) : m_total(seconds), m_remaining(seconds)
{
}

float Lifetime::total() const
{
    return m_total;
}

float Lifetime::remaining() const
{
    return m_remaining;
}

void Lifetime::elapse(float dt)
{
    m_remaining -= dt;
}

} // namespace engine::ecs
