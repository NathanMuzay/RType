#pragma once

#include "Color.hpp"
#include "IComponent.hpp"

namespace engine::ecs {

class ParticleEmitter : public IComponent
{
public:
    ParticleEmitter(int count, float speed, float lifetime, float size, Color color);

    int count() const;
    float speed() const;
    float lifetime() const;
    float size() const;
    Color color() const;
    bool emitted() const;
    void markEmitted();

private:
    int m_count;
    float m_speed;
    float m_lifetime;
    float m_size;
    Color m_color;
    bool m_emitted;
};

} // namespace engine::ecs
