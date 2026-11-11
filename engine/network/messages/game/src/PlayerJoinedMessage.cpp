#include "PlayerJoinedMessage.hpp"

namespace engine::network {

PlayerJoinedMessage::PlayerJoinedMessage(std::uint8_t playerId) : m_playerId(playerId)
{
}

std::uint8_t PlayerJoinedMessage::playerId() const
{
    return m_playerId;
}

bool PlayerJoinedMessage::isValid(const NetworkSettings &settings) const
{
    return m_playerId < settings.maxPlayers();
}

void PlayerJoinedMessage::write(engine::net::ByteWriter &writer) const
{
    writer.writeU8(m_playerId);
}

PlayerJoinedMessage PlayerJoinedMessage::read(engine::net::ByteReader &reader)
{
    return PlayerJoinedMessage(reader.readU8());
}

} // namespace engine::network
