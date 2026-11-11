#include "UdpTransportImpl.hpp"

#include <functional>
#include <utility>

#include "NetError.hpp"

namespace engine::net {

UdpTransport::Impl::Impl(std::uint16_t port)
    : m_socket(m_io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), m_buffer(), m_pending(0)
{
    receive();
    m_worker = std::thread(&Impl::runIo, this);
}

UdpTransport::Impl::~Impl()
{
    const auto deadline = std::chrono::steady_clock::now() + FLUSH_TIMEOUT;
    while (m_pending.load() > 0 && std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(FLUSH_POLL);
    m_io.stop();
    if (m_worker.joinable())
        m_worker.join();
}

void UdpTransport::Impl::send(const Endpoint &peer, std::vector<std::uint8_t> bytes)
{
    const asio::ip::udp::endpoint destination(asio::ip::make_address_v4(peer.address()), peer.port());
    if (m_pending.fetch_add(1) >= MAX_QUEUE) {
        m_pending.fetch_sub(1);
        return;
    }
    asio::post(m_io,
        std::bind_front(
            &Impl::startSend, this, destination, std::make_shared<std::vector<std::uint8_t>>(std::move(bytes))));
}

std::vector<Datagram> UdpTransport::Impl::takeIncoming()
{
    std::lock_guard lock(m_mutex);
    std::vector<Datagram> incoming;
    incoming.swap(m_incoming);
    return incoming;
}

void UdpTransport::Impl::checkHealth() const
{
    std::lock_guard lock(m_mutex);
    if (m_failure)
        std::rethrow_exception(m_failure);
}

void UdpTransport::Impl::runIo()
{
    try {
        m_io.run();
    } catch (const std::exception &error) {
        fail(error.what());
    } catch (...) {
        fail("unknown network failure");
    }
}

void UdpTransport::Impl::receive()
{
    m_socket.async_receive_from(asio::buffer(m_buffer), m_sender, std::bind_front(&Impl::onReceive, this));
}

void UdpTransport::Impl::onReceive(const asio::error_code &error, std::size_t size)
{
    if (!error && size <= MAX_DATAGRAM_SIZE)
        store(size);
    if (error != asio::error::operation_aborted)
        receive();
}

void UdpTransport::Impl::store(std::size_t size)
{
    std::lock_guard lock(m_mutex);
    if (m_incoming.size() >= MAX_QUEUE)
        return;
    m_incoming.emplace_back(Endpoint(m_sender.address().to_string(), m_sender.port()),
        std::vector<std::uint8_t>(m_buffer.begin(), m_buffer.begin() + static_cast<std::ptrdiff_t>(size)));
}

void UdpTransport::Impl::startSend(asio::ip::udp::endpoint destination, Bytes data)
{
    m_socket.async_send_to(asio::buffer(*data), destination, std::bind_front(&Impl::onSent, this, data));
}

void UdpTransport::Impl::onSent(Bytes, const asio::error_code &, std::size_t)
{
    m_pending.fetch_sub(1);
}

void UdpTransport::Impl::fail(const std::string &reason)
{
    std::lock_guard lock(m_mutex);
    m_failure = std::make_exception_ptr(NetError("UdpTransport: " + reason));
}

} // namespace engine::net
