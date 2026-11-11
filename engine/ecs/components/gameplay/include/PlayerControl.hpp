#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class PlayerControl : public IComponent
{
public:
    explicit PlayerControl(float speed);

    float speed() const;
    void setDirection(bool up, bool down, bool left, bool right);
    float horizontal() const;
    float vertical() const;
    void setFiring(bool firing);
    bool firing() const;
    void setAction(bool action);
    bool action() const;
    bool actionPressed() const;
    void setBomb(bool bomb);
    bool bombPressed() const;
    float fireHeld() const;
    float fireReleased() const;
    void advance(float dt);

private:
    float m_speed;
    float m_horizontal;
    float m_vertical;
    bool m_firing;
    bool m_action;
    bool m_previousAction;
    bool m_actionPressed;
    bool m_bomb;
    bool m_previousBomb;
    bool m_bombPressed;
    float m_fireHeld;
    float m_fireReleased;
};

} // namespace engine::ecs
