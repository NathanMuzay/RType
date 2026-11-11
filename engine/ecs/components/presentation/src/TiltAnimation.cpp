#include "TiltAnimation.hpp"

#include <algorithm>
#include <cmath>

namespace engine::ecs {

TiltAnimation::TiltAnimation(IntRect firstFrame, int row, int flatColumn, int maxTilt, float framesPerSecond)
    : m_firstFrame(firstFrame), m_row(row), m_flatColumn(flatColumn), m_maxTilt(maxTilt),
      m_framesPerSecond(framesPerSecond), m_direction(0.f), m_tilt(0.f)
{
}

void TiltAnimation::setRow(int row)
{
    m_row = row;
}

void TiltAnimation::setDirection(float direction)
{
    m_direction = direction;
}

void TiltAnimation::advance(float dt)
{
    const float target = m_direction * static_cast<float>(m_maxTilt);
    const float step = m_framesPerSecond * dt;
    m_tilt += std::clamp(target - m_tilt, -step, step);
}

IntRect TiltAnimation::currentFrame() const
{
    const int column = m_flatColumn + static_cast<int>(std::lround(m_tilt));
    return {m_firstFrame.x() + column * m_firstFrame.width(), m_firstFrame.y() + m_row * m_firstFrame.height(),
        m_firstFrame.width(), m_firstFrame.height()};
}

} // namespace engine::ecs
