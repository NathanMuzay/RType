#include "HeartbeatAckMessage.hpp"

namespace engine::network {

HeartbeatAckMessage::HeartbeatAckMessage(std::uint32_t timestamp) : m_timestamp(timestamp)
{
}

std::uint32_t HeartbeatAckMessage::timestamp() const
{
    return m_timestamp;
}

bool HeartbeatAckMessage::isValid(const NetworkSettings &) const
{
    return true;
}

void HeartbeatAckMessage::write(engine::net::ByteWriter &writer) const
{
    writer.writeU32(m_timestamp);
}

HeartbeatAckMessage HeartbeatAckMessage::read(engine::net::ByteReader &reader)
{
    return HeartbeatAckMessage(reader.readU32());
}

} // namespace engine::network
