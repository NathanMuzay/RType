#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Datagram.hpp"
#include "Endpoint.hpp"

namespace engine::net {

class UdpTransport
{
public:
    static constexpr std::size_t MAX_DATAGRAM_SIZE = 1200;
    static constexpr const char *ROUTE_PROBE_ADDRESS = "8.8.8.8";
    static constexpr std::uint16_t ROUTE_PROBE_PORT = 53;

    explicit UdpTransport(std::uint16_t port = 0);
    ~UdpTransport();
    UdpTransport(const UdpTransport &) = delete;
    UdpTransport &operator=(const UdpTransport &) = delete;

    void send(const Endpoint &peer, std::vector<std::uint8_t> bytes);
    std::vector<Datagram> poll();
    void checkHealth() const;
    static Endpoint resolve(const std::string &host, std::uint16_t port);
    static std::optional<std::string> primaryLocalAddress();

private:
    class Impl;

    std::unique_ptr<Impl> m_impl;
};

} // namespace engine::net
