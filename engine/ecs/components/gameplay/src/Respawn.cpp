#include "Respawn.hpp"

#include <utility>

namespace engine::ecs {

Respawn::Respawn(
    std::string prefab, float delay, float invulnerability, float x, float y, float spacingX, float spacingY)
    : m_prefab(std::move(prefab)), m_delay(delay), m_invulnerability(invulnerability), m_x(x), m_y(y),
      m_spacingX(spacingX), m_spacingY(spacingY), m_timer(0.f), m_waiting(true), m_firstSpawn(true)
{
}

const std::string &Respawn::prefab() const
{
    return m_prefab;
}

float Respawn::invulnerability() const
{
    return m_invulnerability;
}

float Respawn::x() const
{
    return m_x;
}

float Respawn::y() const
{
    return m_y;
}

float Respawn::spacingX() const
{
    return m_spacingX;
}

float Respawn::spacingY() const
{
    return m_spacingY;
}

float Respawn::timer() const
{
    return m_timer;
}

bool Respawn::waiting() const
{
    return m_waiting;
}

bool Respawn::firstSpawn() const
{
    return m_firstSpawn;
}

void Respawn::schedule()
{
    m_timer = m_delay;
    m_waiting = true;
}

void Respawn::elapse(float dt)
{
    m_timer -= dt;
}

void Respawn::spawned()
{
    m_waiting = false;
    m_firstSpawn = false;
}

void Respawn::reset()
{
    m_timer = 0.f;
    m_waiting = true;
    m_firstSpawn = true;
}

} // namespace engine::ecs
