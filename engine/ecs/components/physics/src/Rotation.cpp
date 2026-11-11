#include "Rotation.hpp"

namespace engine::ecs {

Rotation::Rotation(float degrees) : m_degrees(degrees)
{
}

float Rotation::degrees() const
{
    return m_degrees;
}

void Rotation::setDegrees(float degrees)
{
    m_degrees = degrees;
}

} // namespace engine::ecs
