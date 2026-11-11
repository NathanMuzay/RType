#include "StartGameMessage.hpp"

namespace engine::network {

StartGameMessage::StartGameMessage(std::uint32_t token) : m_token(token)
{
}

std::uint32_t StartGameMessage::token() const
{
    return m_token;
}

bool StartGameMessage::isValid(const NetworkSettings &) const
{
    return m_token != 0;
}

void StartGameMessage::write(net::ByteWriter &writer) const
{
    writer.writeU32(m_token);
}

StartGameMessage StartGameMessage::read(net::ByteReader &reader)
{
    const std::uint32_t token = reader.readU32();
    return StartGameMessage(token);
}

} // namespace engine::network
