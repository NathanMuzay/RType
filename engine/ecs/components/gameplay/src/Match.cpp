#include "Match.hpp"

namespace engine::ecs {

Match::Match(float restartDelay) : m_phase(Phase::Waiting), m_restartDelay(restartDelay), m_timer(0.f)
{
}

Match::Phase Match::phase() const
{
    return m_phase;
}

void Match::setPhase(Phase phase)
{
    m_phase = phase;
}

float Match::timer() const
{
    return m_timer;
}

void Match::startTimer()
{
    m_timer = m_restartDelay;
}

void Match::startTimer(float seconds)
{
    m_timer = seconds;
}

void Match::elapse(float dt)
{
    m_timer -= dt;
}

} // namespace engine::ecs
