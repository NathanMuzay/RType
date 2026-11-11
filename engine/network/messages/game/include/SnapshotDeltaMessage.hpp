#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "EntityChange.hpp"
#include "NetworkSettings.hpp"
#include "PlayerStatus.hpp"
#include "Protocol.hpp"

namespace engine::network {

class SnapshotDeltaMessage
{
public:
    static constexpr MessageType TYPE = MessageType::SnapshotDelta;
    static constexpr std::size_t FIXED_SIZE = 13;
    static constexpr std::size_t WORLD_ENTRY_SIZE = 3;

    using WorldChanges = std::vector<std::pair<std::uint8_t, std::uint16_t>>;
    using PlayerChanges = std::vector<std::pair<std::uint8_t, PlayerStatus>>;

    SnapshotDeltaMessage(std::uint32_t tick, std::uint32_t baselineTick, std::uint8_t gameState, WorldChanges world,
        PlayerChanges players, std::vector<EntityChange> entities, std::vector<std::uint16_t> removed);

    std::uint32_t tick() const;
    std::uint32_t baselineTick() const;
    std::uint8_t gameState() const;
    const WorldChanges &world() const;
    const PlayerChanges &players() const;
    const std::vector<EntityChange> &entities() const;
    const std::vector<std::uint16_t> &removed() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static SnapshotDeltaMessage read(net::ByteReader &reader);

private:
    std::uint32_t m_tick;
    std::uint32_t m_baselineTick;
    std::uint8_t m_gameState;
    WorldChanges m_world;
    PlayerChanges m_players;
    std::vector<EntityChange> m_entities;
    std::vector<std::uint16_t> m_removed;
};

} // namespace engine::network
