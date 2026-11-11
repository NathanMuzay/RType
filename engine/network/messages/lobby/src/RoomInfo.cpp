#include "RoomInfo.hpp"

#include <utility>

namespace engine::network {

RoomInfo::RoomInfo(
    std::uint16_t id, std::string name, std::uint8_t players, std::uint8_t capacity, bool locked, bool playing)
    : m_id(id), m_name(std::move(name)), m_players(players), m_capacity(capacity), m_locked(locked), m_playing(playing)
{
}

std::uint16_t RoomInfo::id() const
{
    return m_id;
}

const std::string &RoomInfo::name() const
{
    return m_name;
}

std::uint8_t RoomInfo::players() const
{
    return m_players;
}

std::uint8_t RoomInfo::capacity() const
{
    return m_capacity;
}

bool RoomInfo::locked() const
{
    return m_locked;
}

bool RoomInfo::playing() const
{
    return m_playing;
}

void RoomInfo::write(net::ByteWriter &writer) const
{
    writer.writeU16(m_id);
    writer.writeString(m_name);
    writer.writeU8(m_players);
    writer.writeU8(m_capacity);
    writer.writeU8(static_cast<std::uint8_t>((m_locked ? LOCKED_FLAG : 0) | (m_playing ? PLAYING_FLAG : 0)));
}

RoomInfo RoomInfo::read(net::ByteReader &reader)
{
    const std::uint16_t id = reader.readU16();
    std::string name = reader.readString();
    const std::uint8_t players = reader.readU8();
    const std::uint8_t capacity = reader.readU8();
    const std::uint8_t flags = reader.readU8();
    return RoomInfo(id, std::move(name), players, capacity, (flags & LOCKED_FLAG) != 0, (flags & PLAYING_FLAG) != 0);
}

std::size_t RoomInfo::sizeFor(std::size_t nameLength)
{
    return FIXED_SIZE + nameLength;
}

} // namespace engine::network
