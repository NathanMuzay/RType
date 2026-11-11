#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class Health : public IComponent
{
public:
    explicit Health(int points);

    int points() const;
    int maximum() const;
    void damage(int amount);
    void heal(int amount);
    void set(int points, int maximum);
    bool isDead() const;

private:
    int m_points;
    int m_maximum;
};

} // namespace engine::ecs
