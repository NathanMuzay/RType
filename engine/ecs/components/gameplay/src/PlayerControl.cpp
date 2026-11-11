#include "PlayerControl.hpp"

namespace engine::ecs {

namespace {

float axis(bool positive, bool negative)
{
    float value = 0.f;
    if (positive)
        value += 1.f;
    if (negative)
        value -= 1.f;
    return value;
}

} // namespace

PlayerControl::PlayerControl(float speed)
    : m_speed(speed), m_horizontal(0.f), m_vertical(0.f), m_firing(false), m_action(false), m_previousAction(false),
      m_actionPressed(false), m_bomb(false), m_previousBomb(false), m_bombPressed(false), m_fireHeld(0.f),
      m_fireReleased(0.f)
{
}

float PlayerControl::speed() const
{
    return m_speed;
}

void PlayerControl::setDirection(bool up, bool down, bool left, bool right)
{
    m_horizontal = axis(right, left);
    m_vertical = axis(down, up);
}

float PlayerControl::horizontal() const
{
    return m_horizontal;
}

float PlayerControl::vertical() const
{
    return m_vertical;
}

void PlayerControl::setFiring(bool firing)
{
    m_firing = firing;
}

bool PlayerControl::firing() const
{
    return m_firing;
}

void PlayerControl::setAction(bool action)
{
    m_action = action;
}

bool PlayerControl::action() const
{
    return m_action;
}

bool PlayerControl::actionPressed() const
{
    return m_actionPressed;
}

void PlayerControl::setBomb(bool bomb)
{
    m_bomb = bomb;
}

bool PlayerControl::bombPressed() const
{
    return m_bombPressed;
}

float PlayerControl::fireHeld() const
{
    return m_fireHeld;
}

float PlayerControl::fireReleased() const
{
    return m_fireReleased;
}

void PlayerControl::advance(float dt)
{
    m_actionPressed = m_action && !m_previousAction;
    m_previousAction = m_action;
    m_bombPressed = m_bomb && !m_previousBomb;
    m_previousBomb = m_bomb;
    m_fireReleased = 0.f;
    if (m_firing) {
        m_fireHeld += dt;
    } else if (m_fireHeld > 0.f) {
        m_fireReleased = m_fireHeld;
        m_fireHeld = 0.f;
    }
}

} // namespace engine::ecs
