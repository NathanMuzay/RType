#include "DeathEffectSystem.hpp"

#include <string>

#include "DeathEffect.hpp"
#include "RectShape.hpp"
#include "Sprite.hpp"
#include "Transform.hpp"

namespace engine::ecs {

DeathEffectSystem::DeathEffectSystem(Registry &registry, IEntityFactory &factory, const EventQueue<DeathEvent> &deaths)
    : m_registry(registry), m_factory(factory), m_deaths(deaths)
{
}

void DeathEffectSystem::update(float)
{
    for (const DeathEvent &death : m_deaths.events()) {
        const Entity victim = death.victim();
        if (!m_registry.isAlive(victim) || !m_registry.has<DeathEffect>(victim) || !m_registry.has<Transform>(victim))
            continue;
        const Transform &origin = m_registry.get<Transform>(victim);
        const Vector2 victimHalf = halfSizeOf(victim);
        const float centerX = origin.x() + victimHalf.x();
        const float centerY = origin.y() + victimHalf.y();
        const std::string prefab = m_registry.get<DeathEffect>(victim).prefab();

        const Entity effect = m_factory.spawn(prefab);
        const Vector2 effectHalf = halfSizeOf(effect);
        const float x = centerX - effectHalf.x();
        const float y = centerY - effectHalf.y();
        m_registry.emplace<Transform>(effect, x, y);
    }
}

Vector2 DeathEffectSystem::halfSizeOf(const Entity &entity) const
{
    if (m_registry.has<Sprite>(entity)) {
        const Sprite &sprite = m_registry.get<Sprite>(entity);
        return {static_cast<float>(sprite.source().width()) * sprite.scale() / 2.f,
            static_cast<float>(sprite.source().height()) * sprite.scale() / 2.f};
    }
    if (m_registry.has<RectShape>(entity)) {
        const RectShape &shape = m_registry.get<RectShape>(entity);
        return {shape.width() / 2.f, shape.height() / 2.f};
    }
    return {};
}

} // namespace engine::ecs
