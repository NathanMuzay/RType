#include "PlayerLeftMessage.hpp"

namespace engine::network {

PlayerLeftMessage::PlayerLeftMessage(std::uint8_t playerId, std::uint8_t reason)
    : m_playerId(playerId), m_reason(reason)
{
}

std::uint8_t PlayerLeftMessage::playerId() const
{
    return m_playerId;
}

std::uint8_t PlayerLeftMessage::reason() const
{
    return m_reason;
}

bool PlayerLeftMessage::isValid(const NetworkSettings &settings) const
{
    return m_playerId < settings.maxPlayers()
        && (m_reason == Protocol::LEFT_QUIT || m_reason == Protocol::LEFT_TIMEOUT);
}

void PlayerLeftMessage::write(engine::net::ByteWriter &writer) const
{
    writer.writeU8(m_playerId);
    writer.writeU8(m_reason);
}

PlayerLeftMessage PlayerLeftMessage::read(engine::net::ByteReader &reader)
{
    const std::uint8_t playerId = reader.readU8();
    const std::uint8_t reason = reader.readU8();
    return PlayerLeftMessage(playerId, reason);
}

} // namespace engine::network
