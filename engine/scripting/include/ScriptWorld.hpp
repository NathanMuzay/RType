#pragma once

#include <functional>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "DeathEvent.hpp"
#include "EventQueue.hpp"
#include "IEntityFactory.hpp"
#include "Registry.hpp"
#include "SystemContext.hpp"
#include "Vector2.hpp"
#include "WorldValues.hpp"
#include "WorldValueTable.hpp"

namespace engine::scripting {

class ScriptEntity;

class ScriptWorld
{
public:
    static constexpr const char *FINISHED_FLAG = "finished";

    explicit ScriptWorld(scene::SystemContext &context);

    void advance(float dt);
    float time() const;
    bool every(const std::string &name, float seconds);
    void resetTimers();
    float random(float minimum, float maximum);

    void set(const std::string &name, int value);
    void setLabel(const std::string &name, const std::string &label);
    int get(const std::string &name) const;
    std::string label(const std::string &name) const;

    ScriptEntity spawn(const std::string &prefab, float x, float y);
    bool canSpawn() const;
    int countScripted(const std::string &file) const;
    int playersAlive() const;
    int playerCount() const;
    std::string matchPhase() const;
    void endMatch(float delay);
    void finishGame();
    float width() const;
    float height() const;
    void log(const std::string &text) const;
    std::vector<ScriptEntity> players();
    std::vector<ScriptEntity> inLayer(const std::string &layer);
    std::string rule(const std::string &name) const;
    void playSound(const std::string &name);
    void setPlayerValue(const ecs::Entity &avatar, const std::string &name, int value);
    std::vector<ecs::ConsoleEvent> takeCommands();
    void reply(std::size_t slot, const std::string &text);
    const std::string &consoleScript() const;
    std::optional<ScriptEntity> playerShip(std::size_t slot);
    int playerValue(const ecs::Entity &avatar, const std::string &name) const;

    ecs::Registry &registry() const;
    ecs::IEntityFactory &factory() const;
    void kill(const ecs::Entity &entity);

private:
    std::size_t indexOf(const std::string &name) const;

    ecs::Registry &m_registry;
    scene::SystemContext &m_context;
    ecs::IEntityFactory &m_factory;
    EventQueue<ecs::DeathEvent> &m_deaths;
    std::function<bool()> m_canSpawn;
    ecs::WorldValues &m_values;
    const scene::WorldValueTable &m_table;
    Vector2 m_area;
    float m_time;
    std::unordered_map<std::string, float> m_timers;
    std::mt19937 m_random;
};

} // namespace engine::scripting
