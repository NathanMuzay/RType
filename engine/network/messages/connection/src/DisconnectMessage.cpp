#include "DisconnectMessage.hpp"

namespace engine::network {

DisconnectMessage::DisconnectMessage(std::uint32_t token) : m_token(token)
{
}

std::uint32_t DisconnectMessage::token() const
{
    return m_token;
}

bool DisconnectMessage::isValid(const NetworkSettings &) const
{
    return m_token != 0;
}

void DisconnectMessage::write(engine::net::ByteWriter &writer) const
{
    writer.writeU32(m_token);
}

DisconnectMessage DisconnectMessage::read(engine::net::ByteReader &reader)
{
    return DisconnectMessage(reader.readU32());
}

} // namespace engine::network
