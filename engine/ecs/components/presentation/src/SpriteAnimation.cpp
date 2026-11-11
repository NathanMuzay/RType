#include "SpriteAnimation.hpp"

#include <algorithm>

namespace engine::ecs {

SpriteAnimation::SpriteAnimation(IntRect firstFrame, int frameCount, int stepX, float framesPerSecond, bool loop)
    : m_firstFrame(firstFrame), m_frameCount(std::max(1, frameCount)), m_stepX(stepX),
      m_framesPerSecond(framesPerSecond), m_loop(loop), m_elapsed(0.f)
{
}

void SpriteAnimation::advance(float dt)
{
    m_elapsed += dt;
}

IntRect SpriteAnimation::currentFrame() const
{
    int frame = static_cast<int>(m_elapsed * m_framesPerSecond);
    frame = m_loop ? frame % m_frameCount : std::min(frame, m_frameCount - 1);
    return {m_firstFrame.x() + frame * m_stepX, m_firstFrame.y(), m_firstFrame.width(), m_firstFrame.height()};
}

bool SpriteAnimation::finished() const
{
    return !m_loop && m_elapsed * m_framesPerSecond >= static_cast<float>(m_frameCount);
}

} // namespace engine::ecs
