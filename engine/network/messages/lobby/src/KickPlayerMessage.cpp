#include "KickPlayerMessage.hpp"

namespace engine::network {

KickPlayerMessage::KickPlayerMessage(std::uint32_t token, std::uint8_t slot) : m_token(token), m_slot(slot)
{
}

std::uint32_t KickPlayerMessage::token() const
{
    return m_token;
}

std::uint8_t KickPlayerMessage::slot() const
{
    return m_slot;
}

bool KickPlayerMessage::isValid(const NetworkSettings &settings) const
{
    return m_token != 0 && m_slot < settings.maxPlayers();
}

void KickPlayerMessage::write(net::ByteWriter &writer) const
{
    writer.writeU32(m_token);
    writer.writeU8(m_slot);
}

KickPlayerMessage KickPlayerMessage::read(net::ByteReader &reader)
{
    const std::uint32_t token = reader.readU32();
    const std::uint8_t slot = reader.readU8();
    return KickPlayerMessage(token, slot);
}

} // namespace engine::network
