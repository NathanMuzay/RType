#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include "Entity.hpp"

namespace engine::scripting {

class ScriptWorld;

class ScriptEntity
{
public:
    static constexpr float DEFAULT_ANGLE = 180.f;
    static constexpr int NO_SLOT = -1;
    static constexpr float GROUND_PROBE = 2.f;
    static constexpr const char *INPUT_FIRE = "fire";
    static constexpr const char *INPUT_ACTION = "action";
    static constexpr const char *INPUT_BOMB = "bomb";

    ScriptEntity(ScriptWorld &world, ecs::Entity entity);

    void advance(float dt);
    bool alive() const;
    bool exists() const;
    float age() const;
    bool every(const std::string &name, float seconds);

    float x() const;
    float y() const;
    float width() const;
    float height() const;
    bool onGround() const;
    float centerX() const;
    float centerY() const;
    float vx() const;
    float vy() const;
    void setPosition(float x, float y);
    void setVelocity(float vx, float vy);

    int health() const;
    int maxHealth() const;
    float number(const std::string &name, float fallback) const;
    std::string text(const std::string &name, const std::string &fallback) const;

    void fire(const std::string &prefab, float angle, float speed, float dx, float dy);
    void fireSpread(const std::string &prefab, int count, float spread, float angle, float speed);
    void fireAtPlayer(const std::string &prefab, float speed);
    std::optional<std::pair<float, float>> nearestPlayer() const;
    float angleToPlayer() const;

    ScriptEntity spawn(const std::string &prefab, float dx, float dy);
    void setInvulnerable(float seconds);
    float invulnerableTime() const;
    void destroy();
    void kill();
    ScriptWorld &world() const;
    bool isPlayer() const;
    int playerSlot() const;
    void heal(int amount);
    void setHealth(int points, int maximum);
    void setWeapon(const std::string &prefab, float cooldown, int count, float spread);
    void setWeaponOffset(float dx, float dy);
    float inputHeld(const std::string &name) const;
    bool inputPressed(const std::string &name) const;
    float inputReleased(const std::string &name) const;
    void attachTo(const ScriptEntity &parent, float dx, float dy);
    void detach();
    bool attached() const;
    std::optional<ScriptEntity> parent() const;
    void setAngle(float degrees);
    void setPlayerValue(const std::string &name, int value);
    int playerValue(const std::string &name) const;
    bool operator==(const ScriptEntity &other) const;

private:
    template <class Component>
    const Component *component() const;
    template <class Component>
    Component *liveComponent();

    ScriptWorld *m_world;
    ecs::Entity m_entity;
    float m_age;
    std::unordered_map<std::string, float> m_timers;
};

} // namespace engine::scripting

#include "ScriptEntity.tpp"
