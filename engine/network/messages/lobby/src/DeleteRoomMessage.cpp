#include "DeleteRoomMessage.hpp"

namespace engine::network {

DeleteRoomMessage::DeleteRoomMessage(std::uint32_t token) : m_token(token)
{
}

std::uint32_t DeleteRoomMessage::token() const
{
    return m_token;
}

bool DeleteRoomMessage::isValid(const NetworkSettings &) const
{
    return m_token != 0;
}

void DeleteRoomMessage::write(net::ByteWriter &writer) const
{
    writer.writeU32(m_token);
}

DeleteRoomMessage DeleteRoomMessage::read(net::ByteReader &reader)
{
    const std::uint32_t token = reader.readU32();
    return DeleteRoomMessage(token);
}

} // namespace engine::network
