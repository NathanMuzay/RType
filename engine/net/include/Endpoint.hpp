#pragma once

#include <cstdint>
#include <string>

namespace engine::net {

class Endpoint
{
public:
    Endpoint();
    Endpoint(std::string address, std::uint16_t port);

    const std::string &address() const;
    std::uint16_t port() const;

    bool operator==(const Endpoint &other) const = default;

private:
    std::string m_address;
    std::uint16_t m_port;
};

} // namespace engine::net
