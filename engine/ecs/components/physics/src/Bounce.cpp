#include "Bounce.hpp"

namespace engine::ecs {

Bounce::Bounce(float restitution) : m_restitution(restitution)
{
}

float Bounce::restitution() const
{
    return m_restitution;
}

} // namespace engine::ecs
