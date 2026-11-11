#pragma once

#include <string>

#include "IComponent.hpp"

namespace engine::ecs {

class Weapon : public IComponent
{
public:
    Weapon(std::string prefab, float cooldown, float offsetX, float offsetY, int count, float spread);

    const std::string &prefab() const;
    float offsetX() const;
    float offsetY() const;
    int count() const;
    float spread() const;
    void configure(std::string prefab, float cooldown, int count, float spread);
    void setOffset(float offsetX, float offsetY);
    void elapse(float dt);
    bool ready() const;
    void trigger();

private:
    std::string m_prefab;
    float m_cooldown;
    float m_offsetX;
    float m_offsetY;
    int m_count;
    float m_spread;
    float m_remaining;
};

} // namespace engine::ecs
