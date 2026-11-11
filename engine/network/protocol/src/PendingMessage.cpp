#include "PendingMessage.hpp"

#include <utility>

namespace engine::network {

PendingMessage::PendingMessage(Packet packet, Time sentAt) : m_packet(std::move(packet)), m_sentAt(sentAt)
{
}

const Packet &PendingMessage::packet() const
{
    return m_packet;
}

PendingMessage::Time PendingMessage::sentAt() const
{
    return m_sentAt;
}

} // namespace engine::network
