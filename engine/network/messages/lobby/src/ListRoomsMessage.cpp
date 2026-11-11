#include "ListRoomsMessage.hpp"

namespace engine::network {

ListRoomsMessage::ListRoomsMessage(std::uint32_t token) : m_token(token)
{
}

std::uint32_t ListRoomsMessage::token() const
{
    return m_token;
}

bool ListRoomsMessage::isValid(const NetworkSettings &) const
{
    return m_token != 0;
}

void ListRoomsMessage::write(net::ByteWriter &writer) const
{
    writer.writeU32(m_token);
}

ListRoomsMessage ListRoomsMessage::read(net::ByteReader &reader)
{
    const std::uint32_t token = reader.readU32();
    return ListRoomsMessage(token);
}

} // namespace engine::network
