#include "AcceptMessage.hpp"

namespace engine::network {

AcceptMessage::AcceptMessage(std::uint32_t token, std::uint8_t playerId) : m_token(token), m_playerId(playerId)
{
}

std::uint32_t AcceptMessage::token() const
{
    return m_token;
}

std::uint8_t AcceptMessage::playerId() const
{
    return m_playerId;
}

bool AcceptMessage::isValid(const NetworkSettings &settings) const
{
    return m_token != 0 && m_playerId < settings.maxClients();
}

void AcceptMessage::write(engine::net::ByteWriter &writer) const
{
    writer.writeU32(m_token);
    writer.writeU8(m_playerId);
}

AcceptMessage AcceptMessage::read(engine::net::ByteReader &reader)
{
    const std::uint32_t token = reader.readU32();
    const std::uint8_t playerId = reader.readU8();
    return AcceptMessage(token, playerId);
}

} // namespace engine::network
