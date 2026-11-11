#include "ParticleEmitter.hpp"

namespace engine::ecs {

ParticleEmitter::ParticleEmitter(int count, float speed, float lifetime, float size, Color color)
    : m_count(count), m_speed(speed), m_lifetime(lifetime), m_size(size), m_color(color), m_emitted(false)
{
}

int ParticleEmitter::count() const
{
    return m_count;
}

float ParticleEmitter::speed() const
{
    return m_speed;
}

float ParticleEmitter::lifetime() const
{
    return m_lifetime;
}

float ParticleEmitter::size() const
{
    return m_size;
}

Color ParticleEmitter::color() const
{
    return m_color;
}

bool ParticleEmitter::emitted() const
{
    return m_emitted;
}

void ParticleEmitter::markEmitted()
{
    m_emitted = true;
}

} // namespace engine::ecs
