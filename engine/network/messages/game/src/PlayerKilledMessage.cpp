#include "PlayerKilledMessage.hpp"

namespace engine::network {

PlayerKilledMessage::PlayerKilledMessage(std::uint8_t playerId) : m_playerId(playerId)
{
}

std::uint8_t PlayerKilledMessage::playerId() const
{
    return m_playerId;
}

bool PlayerKilledMessage::isValid(const NetworkSettings &settings) const
{
    return m_playerId < settings.maxPlayers();
}

void PlayerKilledMessage::write(engine::net::ByteWriter &writer) const
{
    writer.writeU8(m_playerId);
}

PlayerKilledMessage PlayerKilledMessage::read(engine::net::ByteReader &reader)
{
    return PlayerKilledMessage(reader.readU8());
}

} // namespace engine::network
