#include "PlayerStatus.hpp"

#include <utility>

#include "NetError.hpp"

#include "Protocol.hpp"

namespace engine::network {

PlayerStatus::PlayerStatus()
    : m_state(Protocol::SLOT_EMPTY), m_lives(0), m_health(0), m_score(0), m_avatarId(0), m_values()
{
}

PlayerStatus::PlayerStatus(std::uint8_t state, std::uint8_t lives, std::uint8_t health, std::uint32_t score,
    std::uint16_t avatarId, std::vector<std::uint16_t> values)
    : m_state(state), m_lives(lives), m_health(health), m_score(score), m_avatarId(avatarId),
      m_values(std::move(values))
{
}

std::uint8_t PlayerStatus::state() const
{
    return m_state;
}

std::uint8_t PlayerStatus::lives() const
{
    return m_lives;
}

std::uint8_t PlayerStatus::health() const
{
    return m_health;
}

std::uint32_t PlayerStatus::score() const
{
    return m_score;
}

std::uint16_t PlayerStatus::avatarId() const
{
    return m_avatarId;
}

const std::vector<std::uint16_t> &PlayerStatus::values() const
{
    return m_values;
}

std::size_t PlayerStatus::sizeFor(std::size_t values)
{
    return SIZE + values * VALUE_SIZE;
}

bool PlayerStatus::operator==(const PlayerStatus &other) const
{
    return m_state == other.m_state && m_lives == other.m_lives && m_health == other.m_health
        && m_score == other.m_score && m_avatarId == other.m_avatarId && m_values == other.m_values;
}

void PlayerStatus::write(engine::net::ByteWriter &writer) const
{
    writer.writeU8(m_state);
    writer.writeU8(m_lives);
    writer.writeU8(m_health);
    writer.writeU32(m_score);
    writer.writeU16(m_avatarId);
    writer.writeU8(static_cast<std::uint8_t>(m_values.size()));
    for (std::uint16_t value : m_values)
        writer.writeU16(value);
}

PlayerStatus PlayerStatus::read(engine::net::ByteReader &reader)
{
    const std::uint8_t state = reader.readU8();
    const std::uint8_t lives = reader.readU8();
    const std::uint8_t health = reader.readU8();
    const std::uint32_t score = reader.readU32();
    const std::uint16_t avatarId = reader.readU16();
    const std::size_t count = reader.readU8();
    if (count > Protocol::MAX_WORLD_VALUES)
        throw net::NetError("PlayerStatus: too many player values");
    std::vector<std::uint16_t> values;
    values.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
        values.push_back(reader.readU16());
    return PlayerStatus(state, lives, health, score, avatarId, std::move(values));
}

} // namespace engine::network
