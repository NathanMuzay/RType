#pragma once

#include <filesystem>
#include <memory>

#include "CollisionEvent.hpp"
#include "DeathEvent.hpp"
#include "EventQueue.hpp"
#include "ISystem.hpp"
#include "ScriptEngine.hpp"
#include "ScriptWorld.hpp"

namespace engine::scripting {

class ScriptSystemImpl;

class ScriptSystem : public ecs::ISystem
{
public:
    ScriptSystem(ScriptEngine &engine, ScriptWorld world, const EventQueue<ecs::DeathEvent> &deaths,
        const EventQueue<ecs::CollisionEvent> &collisions, std::filesystem::path directory);
    ~ScriptSystem() override;

    void update(float dt) override;

private:
    std::unique_ptr<ScriptSystemImpl> m_impl;
};

} // namespace engine::scripting
