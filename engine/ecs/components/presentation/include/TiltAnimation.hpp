#pragma once

#include "IComponent.hpp"
#include "IntRect.hpp"

namespace engine::ecs {

class TiltAnimation : public IComponent
{
public:
    TiltAnimation(IntRect firstFrame, int row, int flatColumn, int maxTilt, float framesPerSecond);

    void setRow(int row);
    void setDirection(float direction);
    void advance(float dt);
    IntRect currentFrame() const;

private:
    IntRect m_firstFrame;
    int m_row;
    int m_flatColumn;
    int m_maxTilt;
    float m_framesPerSecond;
    float m_direction;
    float m_tilt;
};

} // namespace engine::ecs
