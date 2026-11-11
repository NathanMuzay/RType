#include "DamageSystem.hpp"

#include "Collider.hpp"
#include "ContactDamage.hpp"
#include "Destroyed.hpp"
#include "Health.hpp"
#include "HitFlash.hpp"
#include "Invulnerable.hpp"
#include "Owner.hpp"

namespace engine::ecs {

DamageSystem::DamageSystem(Registry &registry, EventQueue<CollisionEvent> &collisions, EventQueue<DeathEvent> &deaths)
    : m_registry(registry), m_collisions(collisions), m_deaths(deaths)
{
}

void DamageSystem::update(float)
{
    for (const CollisionEvent &collision : m_collisions.events()) {
        apply(collision.first(), collision.second());
        apply(collision.second(), collision.first());
    }
}

void DamageSystem::apply(const Entity &source, const Entity &target)
{
    if (!canInteract(source, target))
        return;
    const ContactDamage &damage = m_registry.get<ContactDamage>(source);
    Health &health = m_registry.get<Health>(target);
    health.damage(damage.amount());
    m_registry.emplace<HitFlash>(target, HitFlash::DEFAULT_SECONDS);
    if (damage.destroyOnHit()) {
        if (m_registry.has<Health>(source))
            markDestroyed(source, std::nullopt);
        else
            m_registry.emplace<Destroyed>(source);
    }
    if (health.isDead())
        markDestroyed(target, creditOf(source));
}

bool DamageSystem::canInteract(const Entity &source, const Entity &target) const
{
    if (!m_registry.isAlive(source) || !m_registry.isAlive(target))
        return false;
    if (m_registry.has<Destroyed>(source) || m_registry.has<Destroyed>(target))
        return false;
    if (!m_registry.has<ContactDamage>(source) || !m_registry.has<Health>(target))
        return false;
    if (m_registry.has<Invulnerable>(target))
        return false;
    return m_registry.get<Collider>(source).canHit(m_registry.get<Collider>(target));
}

void DamageSystem::markDestroyed(const Entity &entity, std::optional<Entity> killer)
{
    m_registry.emplace<Destroyed>(entity);
    m_deaths.push(DeathEvent(entity, killer));
}

Entity DamageSystem::creditOf(const Entity &source) const
{
    if (m_registry.has<Owner>(source))
        return m_registry.get<Owner>(source).owner();
    return source;
}

} // namespace engine::ecs
