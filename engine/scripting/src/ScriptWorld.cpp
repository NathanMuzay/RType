#include "ScriptWorld.hpp"

#include <iostream>
#include <utility>

#include "Collider.hpp"
#include "Destroyed.hpp"
#include "Match.hpp"
#include "MatchSystem.hpp"
#include "Owner.hpp"
#include "Player.hpp"
#include "PlayerControl.hpp"
#include "PlayerValues.hpp"
#include "Script.hpp"
#include "ScriptEntity.hpp"
#include "ScriptError.hpp"
#include "Transform.hpp"

namespace engine::scripting {

ScriptWorld::ScriptWorld(scene::SystemContext &context)
    : m_registry(context.registry()), m_context(context), m_factory(context.factory()), m_deaths(context.deaths()),
      m_canSpawn(context.canSpawn()), m_values(context.worldValues()), m_table(context.worldTable()),
      m_area(context.area()), m_time(0.f), m_timers(), m_random(std::random_device{}())
{
}

void ScriptWorld::advance(float dt)
{
    m_time += dt;
    for (auto &timer : m_timers)
        timer.second += dt;
}

float ScriptWorld::time() const
{
    return m_time;
}

bool ScriptWorld::every(const std::string &name, float seconds)
{
    const auto [found, created] = m_timers.try_emplace(name, 0.f);
    if (created || found->second < seconds)
        return false;
    found->second -= seconds;
    return true;
}

void ScriptWorld::resetTimers()
{
    m_timers.clear();
}

float ScriptWorld::random(float minimum, float maximum)
{
    if (maximum <= minimum)
        return minimum;
    return std::uniform_real_distribution<float>(minimum, maximum)(m_random);
}

void ScriptWorld::set(const std::string &name, int value)
{
    m_values.set(indexOf(name), value);
}

void ScriptWorld::setLabel(const std::string &name, const std::string &label)
{
    const std::size_t index = indexOf(name);
    const auto value = m_table.valueOf(index, label);
    if (!value)
        throw ScriptError("ScriptWorld: world value " + name + " has no label " + label);
    m_values.set(index, *value);
}

int ScriptWorld::get(const std::string &name) const
{
    return m_values.value(indexOf(name));
}

std::string ScriptWorld::label(const std::string &name) const
{
    const std::size_t index = indexOf(name);
    return m_table.label(index, m_values.value(index));
}

ScriptEntity ScriptWorld::spawn(const std::string &prefab, float x, float y)
{
    const ecs::Entity entity = m_factory.spawn(prefab);
    m_registry.emplace<ecs::Transform>(entity, x, y);
    return ScriptEntity(*this, entity);
}

bool ScriptWorld::canSpawn() const
{
    return m_canSpawn();
}

int ScriptWorld::countScripted(const std::string &file) const
{
    int count = 0;
    const auto &scripts = m_registry.getComponents<ecs::Script>();
    for (std::size_t i : m_registry.view<ecs::Script>()) {
        if (scripts.at(i).file() == file && !m_registry.has<ecs::Destroyed>(m_registry.entityAt(i)))
            ++count;
    }
    return count;
}

int ScriptWorld::playersAlive() const
{
    int count = 0;
    for (std::size_t i : m_registry.view<ecs::PlayerControl>()) {
        if (!m_registry.has<ecs::Destroyed>(m_registry.entityAt(i)))
            ++count;
    }
    return count;
}

std::string ScriptWorld::matchPhase() const
{
    const ecs::Match::Phase phase = ecs::MatchSystem::phaseOf(m_registry);
    if (phase == ecs::Match::Phase::Playing)
        return "playing";
    if (phase == ecs::Match::Phase::Over)
        return "over";
    return "waiting";
}

void ScriptWorld::endMatch(float delay)
{
    auto &matches = m_registry.getComponents<ecs::Match>();
    for (std::size_t i : m_registry.view<ecs::Match>()) {
        if (matches.at(i).phase() != ecs::Match::Phase::Playing)
            continue;
        matches.at(i).setPhase(ecs::Match::Phase::Over);
        if (delay > 0.f)
            matches.at(i).startTimer(delay);
        else
            matches.at(i).startTimer();
    }
}

void ScriptWorld::finishGame()
{
    m_context.setFlag(FINISHED_FLAG, true);
}

int ScriptWorld::playerCount() const
{
    int count = 0;
    for (std::size_t i : m_registry.view<ecs::Player>()) {
        if (!m_registry.has<ecs::Destroyed>(m_registry.entityAt(i)))
            ++count;
    }
    return count;
}

float ScriptWorld::width() const
{
    return m_area.x();
}

float ScriptWorld::height() const
{
    return m_area.y();
}

void ScriptWorld::log(const std::string &text) const
{
    std::cout << "[script] " << text << std::endl;
}

ecs::Registry &ScriptWorld::registry() const
{
    return m_registry;
}

ecs::IEntityFactory &ScriptWorld::factory() const
{
    return m_factory;
}

void ScriptWorld::kill(const ecs::Entity &entity)
{
    if (!ecs::Destroyed::isActive(m_registry, entity))
        return;
    m_registry.emplace<ecs::Destroyed>(entity);
    m_deaths.push(ecs::DeathEvent(entity, std::nullopt));
}

std::size_t ScriptWorld::indexOf(const std::string &name) const
{
    const auto index = m_table.index(name);
    if (!index)
        throw ScriptError("ScriptWorld: unknown world value " + name);
    return *index;
}

std::vector<ScriptEntity> ScriptWorld::players()
{
    std::vector<ScriptEntity> ships;
    for (std::size_t i : m_registry.view<ecs::PlayerControl>()) {
        const ecs::Entity ship = m_registry.entityAt(i);
        if (!m_registry.has<ecs::Destroyed>(ship))
            ships.emplace_back(*this, ship);
    }
    return ships;
}

std::vector<ScriptEntity> ScriptWorld::inLayer(const std::string &layer)
{
    const std::uint32_t bit = m_context.layerTable().bit(layer);
    const auto &colliders = m_registry.getComponents<ecs::Collider>();
    std::vector<ScriptEntity> found;
    for (std::size_t i : m_registry.view<ecs::Collider>()) {
        const ecs::Entity entity = m_registry.entityAt(i);
        if (colliders.at(i).layer() == bit && !m_registry.has<ecs::Destroyed>(entity))
            found.emplace_back(*this, entity);
    }
    return found;
}

std::string ScriptWorld::rule(const std::string &name) const
{
    const scene::WorldValueTable &rules = m_context.ruleTable();
    const auto index = rules.index(name);
    if (!index)
        throw ScriptError("ScriptWorld: unknown rule " + name);
    const auto &values = m_context.ruleValues();
    const int value = *index < values.size() ? values[*index] : 0;
    return rules.label(*index, value);
}

void ScriptWorld::playSound(const std::string &name)
{
    m_context.sounds().push(ecs::SoundEvent(name));
}

void ScriptWorld::setPlayerValue(const ecs::Entity &avatar, const std::string &name, int value)
{
    if (!m_registry.isAlive(avatar) || !m_registry.has<ecs::Owner>(avatar))
        return;
    const ecs::Entity owner = m_registry.get<ecs::Owner>(avatar).owner();
    const auto index = m_context.playerTable().index(name);
    if (!index)
        throw ScriptError("ScriptWorld: unknown player value " + name);
    if (m_registry.isAlive(owner) && m_registry.has<ecs::PlayerValues>(owner))
        m_registry.get<ecs::PlayerValues>(owner).set(*index, value);
}

std::vector<ecs::ConsoleEvent> ScriptWorld::takeCommands()
{
    return m_context.commands().take();
}

void ScriptWorld::reply(std::size_t slot, const std::string &text)
{
    m_context.replies().push(ecs::ConsoleEvent(slot, text));
}

const std::string &ScriptWorld::consoleScript() const
{
    return m_context.consoleScript();
}

std::optional<ScriptEntity> ScriptWorld::playerShip(std::size_t slot)
{
    for (ScriptEntity &ship : players()) {
        if (ship.playerSlot() == static_cast<int>(slot))
            return ship;
    }
    return std::nullopt;
}

int ScriptWorld::playerValue(const ecs::Entity &avatar, const std::string &name) const
{
    const auto index = m_context.playerTable().index(name);
    if (!index)
        throw ScriptError("ScriptWorld: unknown player value " + name);
    if (!m_registry.isAlive(avatar) || !m_registry.has<ecs::Owner>(avatar))
        return 0;
    const ecs::Entity owner = m_registry.get<ecs::Owner>(avatar).owner();
    if (!m_registry.isAlive(owner) || !m_registry.has<ecs::PlayerValues>(owner))
        return 0;
    return m_registry.get<ecs::PlayerValues>(owner).values().at(*index);
}

} // namespace engine::scripting
