#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

#include "CollisionEvent.hpp"
#include "DeathEvent.hpp"
#include "Entity.hpp"
#include "EventQueue.hpp"
#include "ScriptEngineImpl.hpp"
#include "ScriptRecord.hpp"
#include "ScriptWorld.hpp"

namespace engine::scripting {

class ScriptSystemImpl
{
public:
    static constexpr int GENERATION_SHIFT = 32;

    ScriptSystemImpl(ScriptEngineImpl &engine, ScriptWorld world, const EventQueue<ecs::DeathEvent> &deaths,
        const EventQueue<ecs::CollisionEvent> &collisions, std::filesystem::path directory);
    ~ScriptSystemImpl();

    void update(float dt);

private:
    void runCommands();
    void notifyDeaths();
    void notifyContacts();
    void notifyContact(const ecs::Entity &entity, const ecs::Entity &other);
    void runEntity(const ecs::Entity &entity, float dt);
    ScriptRecord *recordOf(const ecs::Entity &entity, const std::string &file);
    void call(ScriptRecord &record, const char *hook, const pybind11::object &argument);
    void forgetDeadEntities();
    static std::uint64_t keyOf(const ecs::Entity &entity);

    ScriptEngineImpl &m_engine;
    ScriptWorld m_world;
    const EventQueue<ecs::DeathEvent> &m_deaths;
    const EventQueue<ecs::CollisionEvent> &m_collisions;
    std::filesystem::path m_directory;
    std::unordered_map<std::uint64_t, ScriptRecord> m_records;
    std::unordered_map<std::uint64_t, ecs::Entity> m_entities;
};

} // namespace engine::scripting
