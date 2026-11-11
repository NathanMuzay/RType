#include "Parallax.hpp"

namespace engine::ecs {

Parallax::Parallax(float speed, float wrapWidth) : m_speed(speed), m_wrapWidth(wrapWidth)
{
}

float Parallax::speed() const
{
    return m_speed;
}

float Parallax::wrapWidth() const
{
    return m_wrapWidth;
}

} // namespace engine::ecs
