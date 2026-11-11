#include "Gravity.hpp"

namespace engine::ecs {

Gravity::Gravity(float force) : m_force(force)
{
}

float Gravity::force() const
{
    return m_force;
}

} // namespace engine::ecs
