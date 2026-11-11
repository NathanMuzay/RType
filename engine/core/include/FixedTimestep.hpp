#pragma once

namespace engine {

class FixedTimestep
{
public:
    explicit FixedTimestep(float stepSeconds, float maxFrameSeconds = 0.25f);

    void addTime(float frameSeconds);
    bool step();

    float stepSeconds() const;

private:
    float m_step;
    float m_maxFrame;
    float m_accumulator;
};

} // namespace engine
