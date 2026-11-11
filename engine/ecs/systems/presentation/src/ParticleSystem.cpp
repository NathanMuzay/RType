#include "ParticleSystem.hpp"

#include <cmath>
#include <cstddef>
#include <numbers>

#include "Lifetime.hpp"
#include "ParticleEmitter.hpp"
#include "RectShape.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"

namespace engine::ecs {

ParticleSystem::ParticleSystem(Registry &registry) : m_registry(registry), m_random(std::random_device{}())
{
}

void ParticleSystem::update(float)
{
    for (std::size_t i : m_registry.view<ParticleEmitter, Transform>()) {
        const Entity source = m_registry.entityAt(i);
        if (!m_registry.get<ParticleEmitter>(source).emitted())
            emit(source);
    }
}

void ParticleSystem::emit(const Entity &source)
{
    const ParticleEmitter emitter = m_registry.get<ParticleEmitter>(source);
    const Transform origin = m_registry.get<Transform>(source);
    m_registry.get<ParticleEmitter>(source).markEmitted();
    std::uniform_real_distribution<float> angle(0.f, 2.f * std::numbers::pi_v<float>);
    std::uniform_real_distribution<float> speed(emitter.speed() * 0.3f, emitter.speed());
    for (int index = 0; index < emitter.count(); ++index) {
        const float direction = angle(m_random);
        const float velocity = speed(m_random);
        const Entity particle = m_registry.spawn();
        m_registry.emplace<Transform>(particle, origin.x(), origin.y());
        m_registry.emplace<Velocity>(particle, std::cos(direction) * velocity, std::sin(direction) * velocity);
        m_registry.emplace<RectShape>(particle, emitter.size(), emitter.size(), emitter.color());
        m_registry.emplace<Lifetime>(particle, emitter.lifetime());
    }
}

} // namespace engine::ecs
