#include "FixedTimestep.hpp"

#include <algorithm>

#include "CoreError.hpp"

namespace engine {

FixedTimestep::FixedTimestep(float stepSeconds, float maxFrameSeconds)
    : m_step(stepSeconds), m_maxFrame(maxFrameSeconds), m_accumulator(0.f)
{
    if (stepSeconds <= 0.f || maxFrameSeconds < stepSeconds)
        throw CoreError("FixedTimestep: invalid step or maximum frame duration");
}

void FixedTimestep::addTime(float frameSeconds)
{
    if (frameSeconds < 0.f)
        throw CoreError("FixedTimestep: negative frame duration");
    m_accumulator += std::min(frameSeconds, m_maxFrame);
}

bool FixedTimestep::step()
{
    if (m_accumulator < m_step)
        return false;
    m_accumulator -= m_step;
    return true;
}

float FixedTimestep::stepSeconds() const
{
    return m_step;
}

} // namespace engine
