#include "SnapshotMessage.hpp"

#include <utility>

#include "NetError.hpp"

namespace engine::network {

SnapshotMessage::SnapshotMessage(
    std::uint32_t tick, std::uint8_t gameState, Values values, Players players, std::vector<EntityState> entities)
    : m_tick(tick), m_gameState(gameState), m_values(std::move(values)), m_players(std::move(players)),
      m_entities(std::move(entities))
{
}

std::uint32_t SnapshotMessage::tick() const
{
    return m_tick;
}

std::uint8_t SnapshotMessage::gameState() const
{
    return m_gameState;
}

const SnapshotMessage::Values &SnapshotMessage::values() const
{
    return m_values;
}

const SnapshotMessage::Players &SnapshotMessage::players() const
{
    return m_players;
}

const std::vector<EntityState> &SnapshotMessage::entities() const
{
    return m_entities;
}

bool SnapshotMessage::isValid(const NetworkSettings &settings) const
{
    if (m_gameState > Protocol::GAME_OVER || m_values.size() != settings.worldValueCount()
        || m_entities.size() > settings.maxEntities() || m_players.size() != settings.maxPlayers())
        return false;
    std::unordered_set<std::uint16_t> ids;
    for (const EntityState &entity : m_entities) {
        if (entity.id() == 0 || !ids.insert(entity.id()).second)
            return false;
    }
    std::unordered_set<std::uint16_t> avatars;
    for (const PlayerStatus &player : m_players) {
        if (!validPlayer(player, avatars, settings))
            return false;
    }
    return true;
}

void SnapshotMessage::write(net::ByteWriter &writer) const
{
    writer.writeU32(m_tick);
    writer.writeU8(m_gameState);
    writer.writeU8(static_cast<std::uint8_t>(m_values.size()));
    for (std::uint16_t value : m_values)
        writer.writeU16(value);
    writer.writeU8(static_cast<std::uint8_t>(m_players.size()));
    for (const PlayerStatus &player : m_players)
        player.write(writer);
    writer.writeU8(static_cast<std::uint8_t>(m_entities.size()));
    for (const EntityState &entity : m_entities)
        entity.write(writer);
}

SnapshotMessage SnapshotMessage::read(net::ByteReader &reader)
{
    const std::uint32_t tick = reader.readU32();
    const std::uint8_t gameState = reader.readU8();
    const std::size_t valueCount = reader.readU8();
    if (valueCount > Protocol::MAX_WORLD_VALUES)
        throw net::NetError("SnapshotMessage: too many world values");
    Values values;
    values.reserve(valueCount);
    for (std::size_t i = 0; i < valueCount; ++i)
        values.push_back(reader.readU16());
    const std::size_t playerCount = reader.readU8();
    Players players;
    players.reserve(playerCount);
    for (std::size_t i = 0; i < playerCount; ++i)
        players.push_back(PlayerStatus::read(reader));
    const std::size_t entityCount = reader.readU8();
    if (reader.remaining() != entityCount * EntityState::SIZE)
        throw net::NetError("SnapshotMessage: entity count does not match the payload");
    std::vector<EntityState> entities;
    entities.reserve(entityCount);
    for (std::size_t i = 0; i < entityCount; ++i)
        entities.push_back(EntityState::read(reader));
    return SnapshotMessage(tick, gameState, std::move(values), std::move(players), std::move(entities));
}

std::size_t SnapshotMessage::sizeFor(
    std::size_t players, std::size_t entities, std::size_t values, std::size_t playerValues)
{
    return FIXED_SIZE + values * VALUE_SIZE + players * PlayerStatus::sizeFor(playerValues)
        + entities * EntityState::SIZE;
}

bool SnapshotMessage::validPlayer(
    const PlayerStatus &player, std::unordered_set<std::uint16_t> &avatars, const NetworkSettings &settings) const
{
    if (player.state() == Protocol::SLOT_EMPTY)
        return player.lives() == 0 && player.health() == 0 && player.score() == 0 && player.avatarId() == 0
            && player.values().size() == settings.playerValueCount();
    if (player.values().size() != settings.playerValueCount())
        return false;
    if (player.state() == Protocol::SLOT_DEAD)
        return player.avatarId() == 0;
    if (player.state() != Protocol::SLOT_ALIVE || player.health() > Protocol::MAX_PERCENT)
        return false;
    return player.lives() != 0 && avatars.insert(player.avatarId()).second && hasEntity(player.avatarId());
}

bool SnapshotMessage::hasEntity(std::uint16_t id) const
{
    for (const EntityState &entity : m_entities) {
        if (entity.id() == id)
            return true;
    }
    return false;
}

} // namespace engine::network
