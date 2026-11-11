#include "LeaveRoomMessage.hpp"

namespace engine::network {

LeaveRoomMessage::LeaveRoomMessage(std::uint32_t token) : m_token(token)
{
}

std::uint32_t LeaveRoomMessage::token() const
{
    return m_token;
}

bool LeaveRoomMessage::isValid(const NetworkSettings &) const
{
    return m_token != 0;
}

void LeaveRoomMessage::write(net::ByteWriter &writer) const
{
    writer.writeU32(m_token);
}

LeaveRoomMessage LeaveRoomMessage::read(net::ByteReader &reader)
{
    const std::uint32_t token = reader.readU32();
    return LeaveRoomMessage(token);
}

} // namespace engine::network
