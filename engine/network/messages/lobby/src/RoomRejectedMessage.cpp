#include "RoomRejectedMessage.hpp"

namespace engine::network {

RoomRejectedMessage::RoomRejectedMessage(std::uint8_t reason) : m_reason(reason)
{
}

std::uint8_t RoomRejectedMessage::reason() const
{
    return m_reason;
}

bool RoomRejectedMessage::isValid(const NetworkSettings &) const
{
    return m_reason >= Protocol::ROOM_NOT_FOUND && m_reason <= Protocol::ROOM_NOT_ALLOWED;
}

void RoomRejectedMessage::write(net::ByteWriter &writer) const
{
    writer.writeU8(m_reason);
}

RoomRejectedMessage RoomRejectedMessage::read(net::ByteReader &reader)
{
    const std::uint8_t reason = reader.readU8();
    return RoomRejectedMessage(reason);
}

} // namespace engine::network
