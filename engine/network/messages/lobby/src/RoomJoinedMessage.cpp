#include "RoomJoinedMessage.hpp"

#include <utility>

#include "TextRules.hpp"

namespace engine::network {

RoomJoinedMessage::RoomJoinedMessage(std::uint16_t roomId, std::string name, std::uint8_t slot)
    : m_roomId(roomId), m_name(std::move(name)), m_slot(slot)
{
}

std::uint16_t RoomJoinedMessage::roomId() const
{
    return m_roomId;
}

const std::string &RoomJoinedMessage::name() const
{
    return m_name;
}

std::uint8_t RoomJoinedMessage::slot() const
{
    return m_slot;
}

bool RoomJoinedMessage::isValid(const NetworkSettings &settings) const
{
    return m_slot < settings.maxPlayers() && TextRules::isValid(m_name, 1, settings.roomNameMaxLength());
}

void RoomJoinedMessage::write(net::ByteWriter &writer) const
{
    writer.writeU16(m_roomId);
    writer.writeString(m_name);
    writer.writeU8(m_slot);
}

RoomJoinedMessage RoomJoinedMessage::read(net::ByteReader &reader)
{
    const std::uint16_t roomId = reader.readU16();
    std::string name = reader.readString();
    const std::uint8_t slot = reader.readU8();
    return RoomJoinedMessage(roomId, std::move(name), slot);
}

} // namespace engine::network
