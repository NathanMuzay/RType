#include "WeaponSystem.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <utility>

#include "Owner.hpp"
#include "PlayerControl.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"
#include "Weapon.hpp"

namespace engine::ecs {

WeaponSystem::WeaponSystem(Registry &registry, IEntityFactory &factory, std::function<bool()> canSpawn)
    : m_registry(registry), m_factory(factory), m_canSpawn(std::move(canSpawn))
{
}

void WeaponSystem::update(float dt)
{
    for (std::size_t i : m_registry.view<Weapon, PlayerControl, Transform>()) {
        const Entity shooter = m_registry.entityAt(i);
        Weapon &weapon = m_registry.get<Weapon>(shooter);
        weapon.elapse(dt);
        if (!m_registry.get<PlayerControl>(shooter).firing() || !weapon.ready() || !m_canSpawn())
            continue;
        weapon.trigger();
        const Transform &origin = m_registry.get<Transform>(shooter);
        const float x = origin.x() + weapon.offsetX();
        const float y = origin.y() + weapon.offsetY();
        const Entity owner = m_registry.has<Owner>(shooter) ? m_registry.get<Owner>(shooter).owner() : shooter;
        const int count = std::max(1, weapon.count());
        const float step = count > 1 ? weapon.spread() / static_cast<float>(count - 1) : 0.f;
        for (int index = 0; index < count; ++index) {
            const float angle = count > 1 ? -weapon.spread() / 2.f + step * static_cast<float>(index) : 0.f;
            fire(weapon.prefab(), x, y, angle, owner);
        }
    }
}

void WeaponSystem::fire(const std::string &prefab, float x, float y, float angle, const Entity &owner)
{
    const Entity projectile = m_factory.spawn(prefab);
    m_registry.emplace<Transform>(projectile, x, y);
    m_registry.emplace<Owner>(projectile, owner);
    if (angle == 0.f || !m_registry.has<Velocity>(projectile))
        return;
    Velocity &velocity = m_registry.get<Velocity>(projectile);
    const float radians = angle * std::numbers::pi_v<float> / 180.f;
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);
    velocity.set(velocity.x() * cosine - velocity.y() * sine, velocity.x() * sine + velocity.y() * cosine);
}

} // namespace engine::ecs
