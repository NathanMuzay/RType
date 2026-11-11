#include "HeartbeatMessage.hpp"

namespace engine::network {

HeartbeatMessage::HeartbeatMessage(std::uint32_t token, std::uint32_t timestamp)
    : m_token(token), m_timestamp(timestamp)
{
}

std::uint32_t HeartbeatMessage::token() const
{
    return m_token;
}

std::uint32_t HeartbeatMessage::timestamp() const
{
    return m_timestamp;
}

bool HeartbeatMessage::isValid(const NetworkSettings &) const
{
    return m_token != 0;
}

void HeartbeatMessage::write(engine::net::ByteWriter &writer) const
{
    writer.writeU32(m_token);
    writer.writeU32(m_timestamp);
}

HeartbeatMessage HeartbeatMessage::read(engine::net::ByteReader &reader)
{
    const std::uint32_t token = reader.readU32();
    const std::uint32_t timestamp = reader.readU32();
    return HeartbeatMessage(token, timestamp);
}

} // namespace engine::network
