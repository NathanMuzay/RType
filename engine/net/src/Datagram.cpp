#include "Datagram.hpp"

#include <utility>

namespace engine::net {

Datagram::Datagram(Endpoint sender, std::vector<std::uint8_t> bytes)
    : m_sender(std::move(sender)), m_bytes(std::move(bytes))
{
}

const Endpoint &Datagram::sender() const
{
    return m_sender;
}

const std::vector<std::uint8_t> &Datagram::bytes() const
{
    return m_bytes;
}

} // namespace engine::net
