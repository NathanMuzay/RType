#include "Endpoint.hpp"

#include <utility>

namespace engine::net {

Endpoint::Endpoint() : m_port(0)
{
}

Endpoint::Endpoint(std::string address, std::uint16_t port) : m_address(std::move(address)), m_port(port)
{
}

const std::string &Endpoint::address() const
{
    return m_address;
}

std::uint16_t Endpoint::port() const
{
    return m_port;
}

} // namespace engine::net
