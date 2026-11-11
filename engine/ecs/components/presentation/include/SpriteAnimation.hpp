#pragma once

#include "IComponent.hpp"
#include "IntRect.hpp"

namespace engine::ecs {

class SpriteAnimation : public IComponent
{
public:
    SpriteAnimation(IntRect firstFrame, int frameCount, int stepX, float framesPerSecond, bool loop);

    void advance(float dt);
    IntRect currentFrame() const;
    bool finished() const;

private:
    IntRect m_firstFrame;
    int m_frameCount;
    int m_stepX;
    float m_framesPerSecond;
    bool m_loop;
    float m_elapsed;
};

} // namespace engine::ecs
