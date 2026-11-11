#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "CleanupSystem.hpp"
#include "ComponentFactory.hpp"
#include "ConsoleEvent.hpp"
#include "DataNode.hpp"
#include "Entity.hpp"
#include "EntityState.hpp"
#include "IGameSession.hpp"
#include "LayerTable.hpp"
#include "Match.hpp"
#include "NetworkSettings.hpp"
#include "Packet.hpp"
#include "PlayerStatus.hpp"
#include "PluginManager.hpp"
#include "PrefabLibrary.hpp"
#include "Registry.hpp"
#include "ScriptBuilder.hpp"
#include "ScriptEngine.hpp"
#include "SnapshotMessage.hpp"
#include "SystemContext.hpp"
#include "SystemFactory.hpp"
#include "SystemManager.hpp"
#include "WorldValues.hpp"
#include "WorldValueTable.hpp"

namespace engine::simulation {

class Simulation : public network::IGameSession
{
public:
    static constexpr int MAX_REPORTED_LIVES = 255;

    static constexpr const char *DEFAULT_SCRIPT_DIRECTORY = "scripts";
    static constexpr const char *PLUGIN_FOLDER = "plugins";

    Simulation(const std::filesystem::path &dataDirectory, const std::string &game,
        const network::NetworkSettings &settings, scripting::ScriptEngine &scripts,
        const std::vector<std::uint8_t> &rules);

    void addPlayer(std::size_t slot) override;
    void removePlayer(std::size_t slot) override;
    void setInput(std::size_t slot, std::uint8_t buttons) override;
    void command(std::size_t slot, const std::string &text) override;
    void update(float dt) override;
    std::uint32_t tick() const override;
    bool finished() const override;
    network::SnapshotMessage snapshot() const override;
    std::vector<network::Packet> takeEvents() override;

    ecs::Match::Phase phase() const;
    const ecs::Registry &registry() const;
    std::optional<ecs::Entity> playerEntity(std::size_t slot) const;
    std::optional<ecs::Entity> avatarOf(std::size_t slot) const;

private:
    void collectDeaths();
    void assignNetworkIds();
    void reportPhase();
    bool canSpawn() const;
    network::PlayerStatus statusOf(std::size_t slot) const;
    std::uint8_t healthPercent(const ecs::Entity &entity) const;
    static std::uint8_t percent(float ratio);
    std::vector<network::EntityState> collectEntities() const;
    std::optional<network::EntityState> describe(const ecs::Entity &entity) const;
    std::uint16_t networkIdOf(const ecs::Entity &entity) const;
    std::uint16_t nextNetworkId();
    bool networkIdInUse(std::uint16_t networkId) const;
    static std::uint8_t phaseCode(ecs::Match::Phase phase);
    static scene::WorldValueTable readTable(const scene::DataNode &game, const std::string &key);
    void sendSounds();
    void sendReplies();
    void releaseCommands();
    std::uint8_t visualOf(const ecs::Entity &entity) const;
    static std::filesystem::path scriptDirectory(
        const std::filesystem::path &dataDirectory, const scene::DataNode &game);

    network::NetworkSettings m_settings;
    scene::DataNode m_game;
    ecs::Registry m_registry;
    scene::LayerTable m_layers;
    scene::ComponentFactory m_components;
    scene::PrefabLibrary m_prefabs;
    scene::SystemFactory m_systemFactory;
    scene::PluginManager m_plugins;
    scene::SystemContext m_context;
    scene::WorldValueTable m_worldTable;
    scene::WorldValueTable m_playerTable;
    scene::WorldValueTable m_ruleTable;
    scene::WorldValueTable m_soundTable;
    ecs::WorldValues m_worldValues;
    scripting::ScriptBuilder m_scriptBuilder;
    ecs::SystemManager m_systems;
    ecs::CleanupSystem m_cleanup;
    std::string m_playerPrefab;
    std::vector<network::Packet> m_events;
    ecs::Match::Phase m_reportedPhase;
    std::uint16_t m_nextNetworkId;
    std::uint32_t m_tick;
    std::vector<ecs::ConsoleEvent> m_waitingCommands;
};

} // namespace engine::simulation
