#include "Simulation.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <utility>

#include "ComponentRegistration.hpp"
#include "ConsoleTextMessage.hpp"
#include "Destroyed.hpp"
#include "EngineComponentLoaders.hpp"
#include "EngineSystemBuilders.hpp"
#include "Health.hpp"
#include "HitFlash.hpp"
#include "Lives.hpp"
#include "MatchSystem.hpp"
#include "Player.hpp"
#include "PlayerControl.hpp"
#include "PlayerValues.hpp"
#include "PlaySoundMessage.hpp"
#include "Protocol.hpp"
#include "Replicated.hpp"
#include "Rotation.hpp"
#include "SceneBuilder.hpp"
#include "Score.hpp"
#include "ScriptWorld.hpp"
#include "SimulationError.hpp"
#include "SoundEvent.hpp"
#include "Transform.hpp"

namespace engine::simulation {

Simulation::Simulation(const std::filesystem::path &dataDirectory, const std::string &game,
    const network::NetworkSettings &settings, scripting::ScriptEngine &scripts, const std::vector<std::uint8_t> &rules)
    : m_settings(settings), m_game(scene::DataNode::fromFile(dataDirectory / (game + "_game.json"))),
      m_prefabs(m_registry, m_components), m_plugins(m_systemFactory, dataDirectory.parent_path() / PLUGIN_FOLDER),
      m_context(m_registry, m_prefabs), m_worldTable(readTable(m_game, "worldValues")),
      m_playerTable(readTable(m_game, "playerValues")), m_ruleTable(readTable(m_game, "rules")),
      m_soundTable(readTable(m_game, "eventSounds")), m_worldValues(m_worldTable.size()),
      m_scriptBuilder(scripts, scriptDirectory(dataDirectory, m_game)), m_cleanup(m_registry),
      m_playerPrefab(m_game.getString("playerPrefab")), m_events(), m_reportedPhase(ecs::Match::Phase::Waiting),
      m_nextNetworkId(1), m_tick(0), m_waitingCommands()
{
    ecs::registerEngineComponents(m_registry);
    m_layers.load(m_game.child("layers"));
    scene::EngineComponentLoaders::registerAll(m_components, m_layers);
    scene::EngineSystemBuilders::registerAll(m_systemFactory);
    if (m_game.has("plugins"))
        m_plugins.load(m_game.child("plugins"));
    m_prefabs.load(scene::DataNode::fromFile(dataDirectory / m_game.child("files").getString("serverPrefabs")));
    if (!m_prefabs.has(m_playerPrefab))
        throw SimulationError("Simulation: unknown player prefab " + m_playerPrefab);
    m_context.setCanSpawn(std::bind_front(&Simulation::canSpawn, this));
    m_context.setWorldValues(m_worldValues, m_worldTable);
    m_context.setPlayerTable(m_playerTable);
    m_context.setLayerTable(m_layers);
    m_context.setRules(m_ruleTable, rules);
    if (m_game.child("files").has("console"))
        m_context.setConsoleScript(m_game.child("files").getString("console"));
    m_context.setArea({m_game.child("world").getFloat("width"), m_game.child("world").getFloat("height")});
    m_scriptBuilder.registerInto(m_systemFactory);
    m_systemFactory.build(m_context, m_game.child("serverSystems"), m_systems);
    const scene::DataNode world = m_game.child("world");
    scene::SceneBuilder(m_registry, m_prefabs, m_components, {world.getFloat("width"), world.getFloat("height")})
        .build(m_game.child("serverScene"));
}

void Simulation::addPlayer(std::size_t slot)
{
    if (slot >= m_settings.maxPlayers() || playerEntity(slot))
        throw SimulationError("Simulation: player slot " + std::to_string(slot) + " is invalid or already used");
    const ecs::Entity player = m_prefabs.spawn(m_playerPrefab);
    m_registry.emplace<ecs::Player>(player, slot);
    m_registry.emplace<ecs::PlayerValues>(player, m_playerTable.size());
}

void Simulation::removePlayer(std::size_t slot)
{
    const auto player = playerEntity(slot);
    if (!player)
        return;
    if (const auto avatar = avatarOf(slot))
        m_registry.kill(*avatar);
    m_registry.kill(*player);
}

void Simulation::setInput(std::size_t slot, std::uint8_t buttons)
{
    const auto avatar = avatarOf(slot);
    if (!avatar || !m_registry.has<ecs::PlayerControl>(*avatar))
        return;
    ecs::PlayerControl &control = m_registry.get<ecs::PlayerControl>(*avatar);
    control.setDirection(buttons & network::Protocol::BUTTON_UP, buttons & network::Protocol::BUTTON_DOWN,
        buttons & network::Protocol::BUTTON_LEFT, buttons & network::Protocol::BUTTON_RIGHT);
    control.setFiring(buttons & network::Protocol::BUTTON_FIRE);
    control.setAction(buttons & network::Protocol::BUTTON_ACTION);
    control.setBomb(buttons & network::Protocol::BUTTON_BOMB);
}

void Simulation::update(float dt)
{
    releaseCommands();
    m_systems.update(dt);
    assignNetworkIds();
    collectDeaths();
    sendSounds();
    sendReplies();
    m_context.deaths().clear();
    m_context.collisions().clear();
    m_cleanup.update(dt);
    reportPhase();
    ++m_tick;
}

std::uint32_t Simulation::tick() const
{
    return m_tick;
}

bool Simulation::finished() const
{
    return m_context.flag(scripting::ScriptWorld::FINISHED_FLAG, false);
}

std::vector<network::Packet> Simulation::takeEvents()
{
    return std::exchange(m_events, {});
}

ecs::Match::Phase Simulation::phase() const
{
    return ecs::MatchSystem::phaseOf(m_registry);
}

const ecs::Registry &Simulation::registry() const
{
    return m_registry;
}

std::optional<ecs::Entity> Simulation::playerEntity(std::size_t slot) const
{
    const auto &players = m_registry.getComponents<ecs::Player>();
    for (std::size_t i : m_registry.view<ecs::Player>()) {
        if (players.at(i).slot() == slot)
            return m_registry.entityAt(i);
    }
    return std::nullopt;
}

std::optional<ecs::Entity> Simulation::avatarOf(std::size_t slot) const
{
    const auto player = playerEntity(slot);
    if (!player)
        return std::nullopt;
    const auto avatar = m_registry.get<ecs::Player>(*player).avatar();
    if (!avatar || !ecs::Destroyed::isActive(m_registry, *avatar))
        return std::nullopt;
    return avatar;
}

void Simulation::collectDeaths()
{
    const auto &players = m_registry.getComponents<ecs::Player>();
    for (const ecs::DeathEvent &death : m_context.deaths().events()) {
        const ecs::Entity victim = death.victim();
        const std::uint16_t networkId = networkIdOf(victim);
        if (networkId != 0)
            m_events.emplace_back(network::EntityDestroyedMessage(networkId));
        for (std::size_t i : m_registry.view<ecs::Player>()) {
            if (players.at(i).avatar() != victim)
                continue;
            const std::size_t slot = players.at(i).slot();
            m_events.emplace_back(network::PlayerKilledMessage(static_cast<std::uint8_t>(slot)));
            std::cout << "Player " << slot << " killed" << std::endl;
        }
    }
}

void Simulation::assignNetworkIds()
{
    auto &replicated = m_registry.getComponents<ecs::Replicated>();
    for (std::size_t i : m_registry.view<ecs::Replicated>()) {
        if (replicated.at(i).networkId() == 0)
            replicated.at(i).setNetworkId(nextNetworkId());
    }
}

void Simulation::reportPhase()
{
    const ecs::Match::Phase current = phase();
    if (current == m_reportedPhase)
        return;
    if (current == ecs::Match::Phase::Over)
        std::cout << "Game over" << std::endl;
    else if (current == ecs::Match::Phase::Playing)
        std::cout << "New round" << std::endl;
    m_reportedPhase = current;
}

bool Simulation::canSpawn() const
{
    return phase() == ecs::Match::Phase::Playing
        && m_registry.view<ecs::Replicated>().size() < m_settings.maxEntities();
}

std::uint16_t Simulation::nextNetworkId()
{
    while (m_nextNetworkId == 0 || networkIdInUse(m_nextNetworkId))
        ++m_nextNetworkId;
    return m_nextNetworkId++;
}

bool Simulation::networkIdInUse(std::uint16_t networkId) const
{
    const auto &replicated = m_registry.getComponents<ecs::Replicated>();
    for (std::size_t i : m_registry.view<ecs::Replicated>()) {
        if (replicated.at(i).networkId() == networkId)
            return true;
    }
    return false;
}

scene::WorldValueTable Simulation::readTable(const scene::DataNode &game, const std::string &key)
{
    scene::WorldValueTable table;
    if (game.has(key))
        table.load(game.child(key));
    return table;
}

void Simulation::command(std::size_t slot, const std::string &text)
{
    m_waitingCommands.emplace_back(slot, text);
}

void Simulation::releaseCommands()
{
    std::vector<ecs::ConsoleEvent> waiting;
    for (ecs::ConsoleEvent &command : m_waitingCommands) {
        if (avatarOf(command.slot()) || !playerEntity(command.slot()))
            m_context.commands().push(std::move(command));
        else
            waiting.push_back(std::move(command));
    }
    m_waitingCommands = std::move(waiting);
}

void Simulation::sendReplies()
{
    for (const ecs::ConsoleEvent &reply : m_context.replies().take())
        m_events.emplace_back(network::ConsoleTextMessage(reply.text()));
}

void Simulation::sendSounds()
{
    for (const ecs::SoundEvent &sound : m_context.sounds().take()) {
        if (const auto index = m_soundTable.index(sound.name()))
            m_events.emplace_back(network::PlaySoundMessage(static_cast<std::uint8_t>(*index)));
    }
}

std::filesystem::path Simulation::scriptDirectory(
    const std::filesystem::path &dataDirectory, const scene::DataNode &game)
{
    const scene::DataNode files = game.child("files");
    if (files.has("scripts"))
        return dataDirectory / files.getString("scripts");
    return dataDirectory / DEFAULT_SCRIPT_DIRECTORY;
}

} // namespace engine::simulation
