#include "RejectMessage.hpp"

namespace engine::network {

RejectMessage::RejectMessage(std::uint8_t reason) : m_reason(reason)
{
}

std::uint8_t RejectMessage::reason() const
{
    return m_reason;
}

bool RejectMessage::isValid(const NetworkSettings &) const
{
    return m_reason == Protocol::REJECT_SERVER_FULL || m_reason == Protocol::REJECT_VERSION_MISMATCH;
}

void RejectMessage::write(engine::net::ByteWriter &writer) const
{
    writer.writeU8(m_reason);
}

RejectMessage RejectMessage::read(engine::net::ByteReader &reader)
{
    return RejectMessage(reader.readU8());
}

} // namespace engine::network
