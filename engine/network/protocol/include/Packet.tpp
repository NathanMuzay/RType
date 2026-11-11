#pragma once

#include <utility>

#include "NetError.hpp"

namespace engine::network {

template <class Message>
Packet::Packet(Message message, std::uint32_t sequence, std::uint8_t version)
    : m_type(Message::TYPE), m_version(version), m_sequence(sequence), m_ack(0), m_ackBits(0), m_reliableId(0),
      m_body(std::move(message))
{
}

template <class Message>
bool Packet::holds() const
{
    return std::holds_alternative<Message>(m_body);
}

template <class Message>
const Message &Packet::as() const
{
    if (!holds<Message>())
        throw engine::net::NetError("Packet: the packet does not hold the requested message");
    return std::get<Message>(m_body);
}

template <class Visitor>
decltype(auto) Packet::visit(const Visitor &visitor) const
{
    return std::visit(visitor, m_body);
}

} // namespace engine::network
