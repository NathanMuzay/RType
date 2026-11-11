#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "CollisionEvent.hpp"
#include "ConsoleEvent.hpp"
#include "DeathEvent.hpp"
#include "EventQueue.hpp"
#include "IAudio.hpp"
#include "IEntityFactory.hpp"
#include "IRenderer.hpp"
#include "LayerTable.hpp"
#include "Registry.hpp"
#include "SoundEvent.hpp"
#include "UiActionEvent.hpp"
#include "UiInput.hpp"
#include "UiStore.hpp"
#include "Vector2.hpp"
#include "WorldValues.hpp"
#include "WorldValueTable.hpp"

namespace engine::scene {

class SystemContext
{
public:
    SystemContext(ecs::Registry &registry, ecs::IEntityFactory &factory);

    ecs::Registry &registry() const;
    ecs::IEntityFactory &factory() const;
    EventQueue<ecs::CollisionEvent> &collisions();
    EventQueue<ecs::DeathEvent> &deaths();

    void setCanSpawn(std::function<bool()> canSpawn);
    const std::function<bool()> &canSpawn() const;
    void setRenderer(render::IRenderer &renderer);
    render::IRenderer &renderer() const;
    void setAudio(audio::IAudio &audio);
    audio::IAudio &audio() const;
    void setFlag(const std::string &name, bool value);
    bool flag(const std::string &name, bool fallback) const;
    void setUiInput(const UiInput &input);
    const UiInput &uiInput() const;
    void setUiStore(ecs::UiStore &store);
    ecs::UiStore &uiStore() const;
    EventQueue<ecs::UiActionEvent> &uiActions();
    void setWorldValues(ecs::WorldValues &values, const WorldValueTable &table);
    ecs::WorldValues &worldValues() const;
    const WorldValueTable &worldTable() const;
    EventQueue<ecs::SoundEvent> &sounds();
    EventQueue<ecs::ConsoleEvent> &commands();
    EventQueue<ecs::ConsoleEvent> &replies();
    void setConsoleScript(std::string file);
    const std::string &consoleScript() const;
    void setPlayerTable(const WorldValueTable &table);
    const WorldValueTable &playerTable() const;
    void setLayerTable(const LayerTable &layers);
    const LayerTable &layerTable() const;
    void setRules(const WorldValueTable &table, std::vector<std::uint8_t> values);
    const WorldValueTable &ruleTable() const;
    const std::vector<std::uint8_t> &ruleValues() const;
    void setArea(Vector2 area);
    Vector2 area() const;

private:
    static bool alwaysAllowSpawn();

    ecs::Registry &m_registry;
    ecs::IEntityFactory &m_factory;
    EventQueue<ecs::CollisionEvent> m_collisions;
    EventQueue<ecs::DeathEvent> m_deaths;
    EventQueue<ecs::UiActionEvent> m_uiActions;
    std::function<bool()> m_canSpawn;
    render::IRenderer *m_renderer;
    audio::IAudio *m_audio;
    const UiInput *m_uiInput;
    ecs::UiStore *m_uiStore;
    ecs::WorldValues *m_worldValues;
    const WorldValueTable *m_worldTable;
    Vector2 m_area;
    EventQueue<ecs::SoundEvent> m_sounds;
    EventQueue<ecs::ConsoleEvent> m_commands;
    EventQueue<ecs::ConsoleEvent> m_replies;
    std::string m_consoleScript;
    WorldValueTable m_emptyTable;
    const WorldValueTable *m_playerTable;
    const LayerTable *m_layerTable;
    const WorldValueTable *m_ruleTable;
    std::vector<std::uint8_t> m_ruleValues;
    std::unordered_map<std::string, bool> m_flags;
};

} // namespace engine::scene
