#pragma once

#include <string>

#include "IComponent.hpp"

namespace engine::ecs {

class Respawn : public IComponent
{
public:
    Respawn(std::string prefab, float delay, float invulnerability, float x, float y, float spacingX, float spacingY);

    const std::string &prefab() const;
    float invulnerability() const;
    float x() const;
    float y() const;
    float spacingX() const;
    float spacingY() const;
    float timer() const;
    bool waiting() const;
    bool firstSpawn() const;
    void schedule();
    void elapse(float dt);
    void spawned();
    void reset();

private:
    std::string m_prefab;
    float m_delay;
    float m_invulnerability;
    float m_x;
    float m_y;
    float m_spacingX;
    float m_spacingY;
    float m_timer;
    bool m_waiting;
    bool m_firstSpawn;
};

} // namespace engine::ecs
