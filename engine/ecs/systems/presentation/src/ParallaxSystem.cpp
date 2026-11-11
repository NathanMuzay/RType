#include "ParallaxSystem.hpp"

#include <algorithm>
#include <utility>

#include "Parallax.hpp"
#include "RectShape.hpp"
#include "Sprite.hpp"
#include "Transform.hpp"

namespace engine::ecs {

ParallaxSystem::ParallaxSystem(Registry &registry)
    : m_registry(registry), m_store(nullptr), m_boostKey(), m_boostValue(), m_boost(1.f), m_speedFactor(1.f)
{
}

ParallaxSystem::ParallaxSystem(
    Registry &registry, const UiStore &store, std::string boostKey, std::string boostValue, float boost)
    : m_registry(registry), m_store(&store), m_boostKey(std::move(boostKey)), m_boostValue(std::move(boostValue)),
      m_boost(boost), m_speedFactor(1.f)
{
}

void ParallaxSystem::update(float dt)
{
    updateSpeed(dt);
    const auto &shapes = m_registry.getComponents<RectShape>();
    const auto &sprites = m_registry.getComponents<Sprite>();
    for (std::size_t i : m_registry.view<Transform, Parallax, RectShape>())
        scroll(i, shapes.at(i).width(), dt);
    for (std::size_t i : m_registry.view<Transform, Parallax, Sprite>()) {
        if (!m_registry.has<RectShape>(m_registry.entityAt(i)))
            scroll(i, static_cast<float>(sprites.at(i).source().width()) * sprites.at(i).scale(), dt);
    }
}

void ParallaxSystem::updateSpeed(float dt)
{
    if (!m_store)
        return;
    const float target = m_store->value(m_boostKey) == m_boostValue ? m_boost : 1.f;
    m_speedFactor += (target - m_speedFactor) * std::min(1.f, dt * BOOST_EASING);
}

void ParallaxSystem::scroll(std::size_t index, float width, float dt)
{
    Transform &transform = m_registry.getComponents<Transform>().at(index);
    const Parallax &layer = m_registry.getComponents<Parallax>().at(index);
    transform.translate(-layer.speed() * m_speedFactor * dt, 0.f);
    while (transform.x() < -width)
        transform.setPosition(transform.x() + layer.wrapWidth() + width, transform.y());
}

} // namespace engine::ecs
