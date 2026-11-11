#include "SystemContext.hpp"

#include <utility>

#include "SceneError.hpp"

namespace engine::scene {

SystemContext::SystemContext(ecs::Registry &registry, ecs::IEntityFactory &factory)
    : m_registry(registry), m_factory(factory), m_collisions(), m_deaths(), m_uiActions(),
      m_canSpawn(&SystemContext::alwaysAllowSpawn), m_renderer(nullptr), m_audio(nullptr), m_uiInput(nullptr),
      m_uiStore(nullptr), m_worldValues(nullptr), m_worldTable(nullptr), m_area(), m_sounds(), m_commands(),
      m_replies(), m_consoleScript(), m_emptyTable(), m_playerTable(nullptr), m_layerTable(nullptr),
      m_ruleTable(nullptr), m_ruleValues(), m_flags()
{
}

void SystemContext::setUiInput(const UiInput &input)
{
    m_uiInput = &input;
}

const UiInput &SystemContext::uiInput() const
{
    if (!m_uiInput)
        throw SceneError("SystemContext: this program has no user interface input");
    return *m_uiInput;
}

void SystemContext::setUiStore(ecs::UiStore &store)
{
    m_uiStore = &store;
}

ecs::UiStore &SystemContext::uiStore() const
{
    if (!m_uiStore)
        throw SceneError("SystemContext: this program has no user interface data");
    return *m_uiStore;
}

EventQueue<ecs::UiActionEvent> &SystemContext::uiActions()
{
    return m_uiActions;
}

bool SystemContext::alwaysAllowSpawn()
{
    return true;
}

ecs::Registry &SystemContext::registry() const
{
    return m_registry;
}

ecs::IEntityFactory &SystemContext::factory() const
{
    return m_factory;
}

EventQueue<ecs::CollisionEvent> &SystemContext::collisions()
{
    return m_collisions;
}

EventQueue<ecs::DeathEvent> &SystemContext::deaths()
{
    return m_deaths;
}

void SystemContext::setCanSpawn(std::function<bool()> canSpawn)
{
    if (!canSpawn)
        throw SceneError("SystemContext: empty spawn rule");
    m_canSpawn = std::move(canSpawn);
}

const std::function<bool()> &SystemContext::canSpawn() const
{
    return m_canSpawn;
}

void SystemContext::setRenderer(render::IRenderer &renderer)
{
    m_renderer = &renderer;
}

render::IRenderer &SystemContext::renderer() const
{
    if (!m_renderer)
        throw SceneError("SystemContext: this program has no renderer");
    return *m_renderer;
}

void SystemContext::setAudio(audio::IAudio &audio)
{
    m_audio = &audio;
}

audio::IAudio &SystemContext::audio() const
{
    if (!m_audio)
        throw SceneError("SystemContext: this program has no audio");
    return *m_audio;
}

void SystemContext::setFlag(const std::string &name, bool value)
{
    m_flags.insert_or_assign(name, value);
}

bool SystemContext::flag(const std::string &name, bool fallback) const
{
    const auto found = m_flags.find(name);
    return found == m_flags.end() ? fallback : found->second;
}

void SystemContext::setWorldValues(ecs::WorldValues &values, const WorldValueTable &table)
{
    m_worldValues = &values;
    m_worldTable = &table;
}

ecs::WorldValues &SystemContext::worldValues() const
{
    if (!m_worldValues)
        throw SceneError("SystemContext: this program has no world values");
    return *m_worldValues;
}

const WorldValueTable &SystemContext::worldTable() const
{
    if (!m_worldTable)
        throw SceneError("SystemContext: this program has no world values");
    return *m_worldTable;
}

void SystemContext::setArea(Vector2 area)
{
    m_area = area;
}

Vector2 SystemContext::area() const
{
    return m_area;
}

EventQueue<ecs::SoundEvent> &SystemContext::sounds()
{
    return m_sounds;
}

void SystemContext::setPlayerTable(const WorldValueTable &table)
{
    m_playerTable = &table;
}

const WorldValueTable &SystemContext::playerTable() const
{
    return m_playerTable ? *m_playerTable : m_emptyTable;
}

void SystemContext::setLayerTable(const LayerTable &layers)
{
    m_layerTable = &layers;
}

const LayerTable &SystemContext::layerTable() const
{
    if (!m_layerTable)
        throw SceneError("SystemContext: this program has no layer table");
    return *m_layerTable;
}

void SystemContext::setRules(const WorldValueTable &table, std::vector<std::uint8_t> values)
{
    m_ruleTable = &table;
    m_ruleValues = std::move(values);
}

const WorldValueTable &SystemContext::ruleTable() const
{
    return m_ruleTable ? *m_ruleTable : m_emptyTable;
}

const std::vector<std::uint8_t> &SystemContext::ruleValues() const
{
    return m_ruleValues;
}

EventQueue<ecs::ConsoleEvent> &SystemContext::commands()
{
    return m_commands;
}

EventQueue<ecs::ConsoleEvent> &SystemContext::replies()
{
    return m_replies;
}

void SystemContext::setConsoleScript(std::string file)
{
    m_consoleScript = std::move(file);
}

const std::string &SystemContext::consoleScript() const
{
    return m_consoleScript;
}

} // namespace engine::scene
