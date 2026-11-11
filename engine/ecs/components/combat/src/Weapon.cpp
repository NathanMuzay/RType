#include "Weapon.hpp"

#include <algorithm>
#include <utility>

namespace engine::ecs {

Weapon::Weapon(std::string prefab, float cooldown, float offsetX, float offsetY, int count, float spread)
    : m_prefab(std::move(prefab)), m_cooldown(cooldown), m_offsetX(offsetX), m_offsetY(offsetY), m_count(count),
      m_spread(spread), m_remaining(0.f)
{
}

const std::string &Weapon::prefab() const
{
    return m_prefab;
}

float Weapon::offsetX() const
{
    return m_offsetX;
}

float Weapon::offsetY() const
{
    return m_offsetY;
}

void Weapon::elapse(float dt)
{
    m_remaining = std::max(0.f, m_remaining - dt);
}

bool Weapon::ready() const
{
    return m_remaining <= 0.f;
}

void Weapon::trigger()
{
    m_remaining = m_cooldown;
}

int Weapon::count() const
{
    return m_count;
}

float Weapon::spread() const
{
    return m_spread;
}

void Weapon::configure(std::string prefab, float cooldown, int count, float spread)
{
    m_prefab = std::move(prefab);
    m_cooldown = cooldown;
    m_count = count;
    m_spread = spread;
}

void Weapon::setOffset(float offsetX, float offsetY)
{
    m_offsetX = offsetX;
    m_offsetY = offsetY;
}

} // namespace engine::ecs
