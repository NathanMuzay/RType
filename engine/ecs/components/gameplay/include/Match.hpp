#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class Match : public IComponent
{
public:
    enum class Phase
    {
        Waiting,
        Playing,
        Over
    };

    explicit Match(float restartDelay);

    Phase phase() const;
    void setPhase(Phase phase);
    float timer() const;
    void startTimer();
    void startTimer(float seconds);
    void elapse(float dt);

private:
    Phase m_phase;
    float m_restartDelay;
    float m_timer;
};

} // namespace engine::ecs
