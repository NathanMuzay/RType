#include "RoomListMessage.hpp"

#include <cstddef>
#include <utility>

#include "TextRules.hpp"

namespace engine::network {

RoomListMessage::RoomListMessage(std::vector<RoomInfo> rooms) : m_rooms(std::move(rooms))
{
}

const std::vector<RoomInfo> &RoomListMessage::rooms() const
{
    return m_rooms;
}

bool RoomListMessage::isValid(const NetworkSettings &settings) const
{
    if (m_rooms.size() > settings.maxRooms())
        return false;
    for (const RoomInfo &room : m_rooms) {
        if (!TextRules::isValid(room.name(), 1, settings.roomNameMaxLength()) || room.capacity() > settings.maxPlayers()
            || room.players() > room.capacity())
            return false;
    }
    return true;
}

void RoomListMessage::write(net::ByteWriter &writer) const
{
    writer.writeU8(static_cast<std::uint8_t>(m_rooms.size()));
    for (const RoomInfo &room : m_rooms)
        room.write(writer);
}

RoomListMessage RoomListMessage::read(net::ByteReader &reader)
{
    const std::size_t roomCount = reader.readU8();
    std::vector<RoomInfo> rooms;
    rooms.reserve(roomCount);
    for (std::size_t i = 0; i < roomCount; ++i)
        rooms.push_back(RoomInfo::read(reader));
    return RoomListMessage(std::move(rooms));
}

} // namespace engine::network
