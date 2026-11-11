#include "UdpTransport.hpp"

#include <utility>

#include "NetError.hpp"
#include "UdpTransportImpl.hpp"

namespace engine::net {

UdpTransport::UdpTransport(std::uint16_t port)
{
    try {
        m_impl = std::make_unique<Impl>(port);
    } catch (const asio::system_error &error) {
        throw NetError("UdpTransport: cannot open UDP port " + std::to_string(port) + ": " + error.what());
    }
}

UdpTransport::~UdpTransport() = default;

Endpoint UdpTransport::resolve(const std::string &host, std::uint16_t port)
{
    asio::error_code error;
    const auto numeric = asio::ip::make_address_v4(host, error);
    if (!error)
        return Endpoint(numeric.to_string(), port);
    asio::io_context io;
    asio::ip::udp::resolver resolver(io);
    const auto results = resolver.resolve(asio::ip::udp::v4(), host, std::to_string(port), error);
    if (error || results.empty())
        throw NetError("UdpTransport: cannot find the address of " + host);
    return Endpoint(results.begin()->endpoint().address().to_string(), port);
}

std::optional<std::string> UdpTransport::primaryLocalAddress()
{
    asio::error_code error;
    asio::io_context io;
    asio::ip::udp::socket socket(io);
    socket.open(asio::ip::udp::v4(), error);
    if (!error)
        socket.connect(
            asio::ip::udp::endpoint(asio::ip::make_address_v4(ROUTE_PROBE_ADDRESS), ROUTE_PROBE_PORT), error);
    if (error)
        return std::nullopt;
    const auto local = socket.local_endpoint(error);
    if (error || local.address().is_loopback())
        return std::nullopt;
    return local.address().to_string();
}

void UdpTransport::send(const Endpoint &peer, std::vector<std::uint8_t> bytes)
{
    checkHealth();
    if (bytes.size() > MAX_DATAGRAM_SIZE)
        throw NetError("UdpTransport: datagram exceeds the maximum size");
    m_impl->send(peer, std::move(bytes));
}

std::vector<Datagram> UdpTransport::poll()
{
    checkHealth();
    return m_impl->takeIncoming();
}

void UdpTransport::checkHealth() const
{
    m_impl->checkHealth();
}

} // namespace engine::net
