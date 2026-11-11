#include "SnapshotDeltaMessage.hpp"

#include <unordered_set>

#include "NetError.hpp"

namespace engine::network {

SnapshotDeltaMessage::SnapshotDeltaMessage(std::uint32_t tick, std::uint32_t baselineTick, std::uint8_t gameState,
    WorldChanges world, PlayerChanges players, std::vector<EntityChange> entities, std::vector<std::uint16_t> removed)
    : m_tick(tick), m_baselineTick(baselineTick), m_gameState(gameState), m_world(std::move(world)),
      m_players(std::move(players)), m_entities(std::move(entities)), m_removed(std::move(removed))
{
}

std::uint32_t SnapshotDeltaMessage::tick() const
{
    return m_tick;
}

std::uint32_t SnapshotDeltaMessage::baselineTick() const
{
    return m_baselineTick;
}

std::uint8_t SnapshotDeltaMessage::gameState() const
{
    return m_gameState;
}

const SnapshotDeltaMessage::WorldChanges &SnapshotDeltaMessage::world() const
{
    return m_world;
}

const SnapshotDeltaMessage::PlayerChanges &SnapshotDeltaMessage::players() const
{
    return m_players;
}

const std::vector<EntityChange> &SnapshotDeltaMessage::entities() const
{
    return m_entities;
}

const std::vector<std::uint16_t> &SnapshotDeltaMessage::removed() const
{
    return m_removed;
}

bool SnapshotDeltaMessage::isValid(const NetworkSettings &settings) const
{
    if (m_gameState > Protocol::GAME_OVER || m_baselineTick == 0)
        return false;
    if (m_world.size() > settings.worldValueCount() || m_players.size() > settings.maxPlayers()
        || m_entities.size() > settings.maxEntities() || m_removed.size() > settings.maxEntities())
        return false;
    std::unordered_set<std::uint8_t> seenWorld;
    for (const auto &entry : m_world) {
        if (entry.first >= settings.worldValueCount() || !seenWorld.insert(entry.first).second)
            return false;
    }
    std::unordered_set<std::uint8_t> seenSlots;
    for (const auto &entry : m_players) {
        if (entry.first >= settings.maxPlayers() || !seenSlots.insert(entry.first).second
            || entry.second.values().size() != settings.playerValueCount())
            return false;
    }
    std::unordered_set<std::uint16_t> seenEntities;
    for (const EntityChange &entity : m_entities) {
        if (!entity.isValid() || !seenEntities.insert(entity.id()).second)
            return false;
    }
    for (std::uint16_t id : m_removed) {
        if (id == 0 || seenEntities.count(id) != 0)
            return false;
    }
    return true;
}

void SnapshotDeltaMessage::write(net::ByteWriter &writer) const
{
    writer.writeU32(m_tick);
    writer.writeU32(m_baselineTick);
    writer.writeU8(m_gameState);
    writer.writeU8(static_cast<std::uint8_t>(m_world.size()));
    for (const auto &entry : m_world) {
        writer.writeU8(entry.first);
        writer.writeU16(entry.second);
    }
    writer.writeU8(static_cast<std::uint8_t>(m_players.size()));
    for (const auto &entry : m_players) {
        writer.writeU8(entry.first);
        entry.second.write(writer);
    }
    writer.writeU8(static_cast<std::uint8_t>(m_entities.size()));
    for (const EntityChange &entity : m_entities)
        entity.write(writer);
    writer.writeU8(static_cast<std::uint8_t>(m_removed.size()));
    for (std::uint16_t id : m_removed)
        writer.writeU16(id);
}

SnapshotDeltaMessage SnapshotDeltaMessage::read(net::ByteReader &reader)
{
    const std::uint32_t tick = reader.readU32();
    const std::uint32_t baselineTick = reader.readU32();
    const std::uint8_t gameState = reader.readU8();
    WorldChanges world;
    const std::size_t worldCount = reader.readU8();
    if (worldCount > Protocol::MAX_WORLD_VALUES)
        throw net::NetError("SnapshotDeltaMessage: too many world values");
    world.reserve(worldCount);
    for (std::size_t i = 0; i < worldCount; ++i) {
        const std::uint8_t index = reader.readU8();
        world.emplace_back(index, reader.readU16());
    }
    PlayerChanges players;
    const std::size_t playerCount = reader.readU8();
    players.reserve(playerCount);
    for (std::size_t i = 0; i < playerCount; ++i) {
        const std::uint8_t slot = reader.readU8();
        players.emplace_back(slot, PlayerStatus::read(reader));
    }
    const std::size_t entityCount = reader.readU8();
    std::vector<EntityChange> entities;
    entities.reserve(entityCount);
    for (std::size_t i = 0; i < entityCount; ++i)
        entities.push_back(EntityChange::read(reader));
    const std::size_t removedCount = reader.readU8();
    if (reader.remaining() != removedCount * sizeof(std::uint16_t))
        throw net::NetError("SnapshotDeltaMessage: removed count does not match the payload");
    std::vector<std::uint16_t> removed;
    removed.reserve(removedCount);
    for (std::size_t i = 0; i < removedCount; ++i)
        removed.push_back(reader.readU16());
    return SnapshotDeltaMessage(
        tick, baselineTick, gameState, std::move(world), std::move(players), std::move(entities), std::move(removed));
}

} // namespace engine::network
