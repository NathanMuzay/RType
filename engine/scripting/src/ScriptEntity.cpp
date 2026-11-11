#include "ScriptEntity.hpp"

#include <cmath>
#include <numbers>

#include "Attachment.hpp"
#include "Collider.hpp"
#include "Destroyed.hpp"
#include "Health.hpp"
#include "Invulnerable.hpp"
#include "Owner.hpp"
#include "Player.hpp"
#include "PlayerControl.hpp"
#include "Rotation.hpp"
#include "Script.hpp"
#include "ScriptError.hpp"
#include "ScriptWorld.hpp"
#include "Solid.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"
#include "Weapon.hpp"

namespace engine::scripting {

ScriptEntity::ScriptEntity(ScriptWorld &world, ecs::Entity entity)
    : m_world(&world), m_entity(entity), m_age(0.f), m_timers()
{
}

void ScriptEntity::advance(float dt)
{
    m_age += dt;
    for (auto &timer : m_timers)
        timer.second += dt;
}

bool ScriptEntity::alive() const
{
    return ecs::Destroyed::isActive(m_world->registry(), m_entity);
}

bool ScriptEntity::exists() const
{
    return m_world->registry().isAlive(m_entity);
}

float ScriptEntity::age() const
{
    return m_age;
}

bool ScriptEntity::every(const std::string &name, float seconds)
{
    const auto [found, created] = m_timers.try_emplace(name, 0.f);
    if (created || found->second < seconds)
        return false;
    found->second -= seconds;
    return true;
}

float ScriptEntity::x() const
{
    const auto *transform = component<ecs::Transform>();
    return transform ? transform->x() : 0.f;
}

float ScriptEntity::y() const
{
    const auto *transform = component<ecs::Transform>();
    return transform ? transform->y() : 0.f;
}

float ScriptEntity::width() const
{
    const auto *collider = component<ecs::Collider>();
    return collider ? collider->width() : 0.f;
}

float ScriptEntity::height() const
{
    const auto *collider = component<ecs::Collider>();
    return collider ? collider->height() : 0.f;
}

bool ScriptEntity::onGround() const
{
    const auto *collider = component<ecs::Collider>();
    const auto *transform = component<ecs::Transform>();
    if (!collider || !transform)
        return false;
    const ecs::Registry &registry = m_world->registry();
    const physics::Aabb probe = collider->boundsAt(transform->x(), transform->y() + GROUND_PROBE);
    for (std::size_t i : registry.view<ecs::Solid, ecs::Transform, ecs::Collider>()) {
        const ecs::Entity wall = registry.entityAt(i);
        if (wall == m_entity)
            continue;
        const ecs::Transform &block = registry.get<ecs::Transform>(wall);
        const auto push = probe.separation(registry.get<ecs::Collider>(wall).boundsAt(block.x(), block.y()));
        if (push && push->second < 0.f)
            return true;
    }
    return false;
}

float ScriptEntity::centerX() const
{
    return x() + width() / 2.f;
}

float ScriptEntity::centerY() const
{
    return y() + height() / 2.f;
}

float ScriptEntity::vx() const
{
    const auto *velocity = component<ecs::Velocity>();
    return velocity ? velocity->x() : 0.f;
}

float ScriptEntity::vy() const
{
    const auto *velocity = component<ecs::Velocity>();
    return velocity ? velocity->y() : 0.f;
}

void ScriptEntity::setPosition(float x, float y)
{
    if (alive())
        m_world->registry().emplace<ecs::Transform>(m_entity, x, y);
}

void ScriptEntity::setVelocity(float vx, float vy)
{
    if (alive())
        m_world->registry().emplace<ecs::Velocity>(m_entity, vx, vy);
}

int ScriptEntity::health() const
{
    const auto *health = component<ecs::Health>();
    return health ? health->points() : 0;
}

int ScriptEntity::maxHealth() const
{
    const auto *health = component<ecs::Health>();
    return health ? health->maximum() : 0;
}

float ScriptEntity::number(const std::string &name, float fallback) const
{
    const auto *script = component<ecs::Script>();
    return script ? script->number(name, fallback) : fallback;
}

std::string ScriptEntity::text(const std::string &name, const std::string &fallback) const
{
    const auto *script = component<ecs::Script>();
    return script ? script->text(name, fallback) : fallback;
}

void ScriptEntity::fire(const std::string &prefab, float angle, float speed, float dx, float dy)
{
    if (!exists() || !m_world->canSpawn())
        return;
    const float originX = centerX() + dx;
    const float originY = centerY() + dy;
    ScriptEntity shot = m_world->spawn(prefab, originX, originY);
    shot.setPosition(originX - shot.width() / 2.f, originY - shot.height() / 2.f);
    const float radians = angle * std::numbers::pi_v<float> / 180.f;
    shot.setVelocity(std::cos(radians) * speed, std::sin(radians) * speed);
    m_world->registry().emplace<ecs::Owner>(shot.m_entity, m_entity);
}

void ScriptEntity::fireSpread(const std::string &prefab, int count, float spread, float angle, float speed)
{
    if (count <= 0)
        return;
    if (count == 1) {
        fire(prefab, angle, speed, 0.f, 0.f);
        return;
    }
    const float step = spread / static_cast<float>(count - 1);
    const float first = angle - spread / 2.f;
    for (int index = 0; index < count; ++index)
        fire(prefab, first + step * static_cast<float>(index), speed, 0.f, 0.f);
}

void ScriptEntity::fireAtPlayer(const std::string &prefab, float speed)
{
    fire(prefab, angleToPlayer(), speed, 0.f, 0.f);
}

std::optional<std::pair<float, float>> ScriptEntity::nearestPlayer() const
{
    const ecs::Registry &registry = m_world->registry();
    std::optional<std::pair<float, float>> nearest;
    float best = 0.f;
    for (std::size_t i : registry.view<ecs::PlayerControl, ecs::Transform>()) {
        const ecs::Entity player = registry.entityAt(i);
        if (registry.has<ecs::Destroyed>(player))
            continue;
        ScriptEntity target(*m_world, player);
        const float distanceX = target.centerX() - centerX();
        const float distanceY = target.centerY() - centerY();
        const float distance = distanceX * distanceX + distanceY * distanceY;
        if (!nearest || distance < best) {
            nearest = std::make_pair(target.centerX(), target.centerY());
            best = distance;
        }
    }
    return nearest;
}

float ScriptEntity::angleToPlayer() const
{
    const auto target = nearestPlayer();
    if (!target)
        return DEFAULT_ANGLE;
    const float radians = std::atan2(target->second - centerY(), target->first - centerX());
    return radians * 180.f / std::numbers::pi_v<float>;
}

ScriptEntity ScriptEntity::spawn(const std::string &prefab, float dx, float dy)
{
    return m_world->spawn(prefab, x() + dx, y() + dy);
}

void ScriptEntity::setInvulnerable(float seconds)
{
    if (!alive())
        return;
    if (seconds <= 0.f) {
        if (component<ecs::Invulnerable>())
            m_world->registry().remove<ecs::Invulnerable>(m_entity);
        return;
    }
    const auto *shield = component<ecs::Invulnerable>();
    if (!shield || shield->remaining() < seconds)
        m_world->registry().emplace<ecs::Invulnerable>(m_entity, seconds);
}

float ScriptEntity::invulnerableTime() const
{
    const auto *shield = component<ecs::Invulnerable>();
    return shield ? shield->remaining() : 0.f;
}

void ScriptEntity::destroy()
{
    if (alive())
        m_world->registry().emplace<ecs::Destroyed>(m_entity);
}

void ScriptEntity::kill()
{
    m_world->kill(m_entity);
}

ScriptWorld &ScriptEntity::world() const
{
    return *m_world;
}

bool ScriptEntity::isPlayer() const
{
    return component<ecs::PlayerControl>() != nullptr;
}

int ScriptEntity::playerSlot() const
{
    const auto *link = component<ecs::Owner>();
    if (!link)
        return NO_SLOT;
    const ecs::Registry &registry = m_world->registry();
    const ecs::Entity owner = link->owner();
    if (!registry.isAlive(owner) || !registry.has<ecs::Player>(owner))
        return NO_SLOT;
    return static_cast<int>(registry.get<ecs::Player>(owner).slot());
}

void ScriptEntity::heal(int amount)
{
    if (auto *health = liveComponent<ecs::Health>())
        health->heal(amount);
}

void ScriptEntity::setHealth(int points, int maximum)
{
    if (auto *health = liveComponent<ecs::Health>())
        health->set(points, maximum);
    else if (alive())
        m_world->registry().emplace<ecs::Health>(m_entity, points);
}

void ScriptEntity::setWeapon(const std::string &prefab, float cooldown, int count, float spread)
{
    if (auto *weapon = liveComponent<ecs::Weapon>())
        weapon->configure(prefab, cooldown, count, spread);
}

void ScriptEntity::setWeaponOffset(float dx, float dy)
{
    if (auto *weapon = liveComponent<ecs::Weapon>())
        weapon->setOffset(dx, dy);
}

float ScriptEntity::inputHeld(const std::string &name) const
{
    const auto *control = alive() ? component<ecs::PlayerControl>() : nullptr;
    if (!control)
        return 0.f;
    if (name == INPUT_FIRE)
        return control->fireHeld();
    if (name == INPUT_ACTION)
        return control->action() ? 1.f : 0.f;
    throw ScriptError("ScriptEntity: unknown input " + name);
}

bool ScriptEntity::inputPressed(const std::string &name) const
{
    const auto *control = alive() ? component<ecs::PlayerControl>() : nullptr;
    if (!control)
        return false;
    if (name == INPUT_ACTION)
        return control->actionPressed();
    if (name == INPUT_BOMB)
        return control->bombPressed();
    if (name == INPUT_FIRE)
        return control->firing();
    throw ScriptError("ScriptEntity: unknown input " + name);
}

float ScriptEntity::inputReleased(const std::string &name) const
{
    if (name != INPUT_FIRE)
        throw ScriptError("ScriptEntity: only fire reports a release, not " + name);
    const auto *control = alive() ? component<ecs::PlayerControl>() : nullptr;
    return control ? control->fireReleased() : 0.f;
}

void ScriptEntity::attachTo(const ScriptEntity &parent, float dx, float dy)
{
    if (alive() && parent.exists())
        m_world->registry().emplace<ecs::Attachment>(m_entity, parent.m_entity, dx, dy);
}

void ScriptEntity::detach()
{
    if (exists() && m_world->registry().has<ecs::Attachment>(m_entity))
        m_world->registry().get<ecs::Attachment>(m_entity).detach();
}

bool ScriptEntity::attached() const
{
    const auto *link = component<ecs::Attachment>();
    return link && link->attached();
}

std::optional<ScriptEntity> ScriptEntity::parent() const
{
    const auto *link = component<ecs::Attachment>();
    if (!link || !m_world->registry().isAlive(link->parent()))
        return std::nullopt;
    return ScriptEntity(*m_world, link->parent());
}

void ScriptEntity::setAngle(float degrees)
{
    if (exists())
        m_world->registry().emplace<ecs::Rotation>(m_entity, degrees);
}

void ScriptEntity::setPlayerValue(const std::string &name, int value)
{
    m_world->setPlayerValue(m_entity, name, value);
}

int ScriptEntity::playerValue(const std::string &name) const
{
    return m_world->playerValue(m_entity, name);
}

bool ScriptEntity::operator==(const ScriptEntity &other) const
{
    return m_entity == other.m_entity;
}

} // namespace engine::scripting
