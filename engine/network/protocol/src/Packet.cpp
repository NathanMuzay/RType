#include "Packet.hpp"

namespace engine::network {

MessageType Packet::type() const
{
    return m_type;
}

std::uint8_t Packet::version() const
{
    return m_version;
}

std::uint32_t Packet::sequence() const
{
    return m_sequence;
}

void Packet::setSequence(std::uint32_t sequence)
{
    m_sequence = sequence;
}

std::uint32_t Packet::ack() const
{
    return m_ack;
}

void Packet::setAck(std::uint32_t ack)
{
    m_ack = ack;
}

std::uint32_t Packet::ackBits() const
{
    return m_ackBits;
}

void Packet::setAckBits(std::uint32_t ackBits)
{
    m_ackBits = ackBits;
}

std::uint16_t Packet::reliableId() const
{
    return m_reliableId;
}

void Packet::setReliableId(std::uint16_t reliableId)
{
    m_reliableId = reliableId;
}

} // namespace engine::network
