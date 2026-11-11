#pragma once

#include <cstddef>
#include <cstdint>
#include <unordered_set>
#include <vector>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "EntityState.hpp"
#include "NetworkSettings.hpp"
#include "PlayerStatus.hpp"
#include "Protocol.hpp"

namespace engine::network {

class SnapshotMessage
{
public:
    static constexpr MessageType TYPE = MessageType::Snapshot;
    static constexpr std::size_t FIXED_SIZE = 8;
    static constexpr std::size_t VALUE_SIZE = 2;

    using Players = std::vector<PlayerStatus>;

    using Values = std::vector<std::uint16_t>;

    SnapshotMessage(
        std::uint32_t tick, std::uint8_t gameState, Values values, Players players, std::vector<EntityState> entities);

    std::uint32_t tick() const;
    std::uint8_t gameState() const;
    const Values &values() const;
    const Players &players() const;
    const std::vector<EntityState> &entities() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static SnapshotMessage read(net::ByteReader &reader);
    static std::size_t sizeFor(std::size_t players, std::size_t entities, std::size_t values, std::size_t playerValues);

private:
    bool validPlayer(
        const PlayerStatus &player, std::unordered_set<std::uint16_t> &avatars, const NetworkSettings &settings) const;
    bool hasEntity(std::uint16_t id) const;

    std::uint32_t m_tick;
    std::uint8_t m_gameState;
    Values m_values;
    Players m_players;
    std::vector<EntityState> m_entities;
};

} // namespace engine::network
