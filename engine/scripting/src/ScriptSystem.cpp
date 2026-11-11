#include "ScriptSystem.hpp"

#include <utility>

#include "ScriptSystemImpl.hpp"

namespace engine::scripting {

ScriptSystem::ScriptSystem(ScriptEngine &engine, ScriptWorld world, const EventQueue<ecs::DeathEvent> &deaths,
    const EventQueue<ecs::CollisionEvent> &collisions, std::filesystem::path directory)
    : m_impl(
          std::make_unique<ScriptSystemImpl>(engine.impl(), std::move(world), deaths, collisions, std::move(directory)))
{
}

ScriptSystem::~ScriptSystem() = default;

void ScriptSystem::update(float dt)
{
    m_impl->update(dt);
}

} // namespace engine::scripting
