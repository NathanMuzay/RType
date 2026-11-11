#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <asio.hpp>

#include "Datagram.hpp"
#include "Endpoint.hpp"
#include "UdpTransport.hpp"

namespace engine::net {

class UdpTransport::Impl
{
public:
    static constexpr std::size_t MAX_QUEUE = 256;
    static constexpr std::chrono::milliseconds FLUSH_TIMEOUT{200};
    static constexpr std::chrono::milliseconds FLUSH_POLL{1};

    explicit Impl(std::uint16_t port);
    ~Impl();
    Impl(const Impl &) = delete;
    Impl &operator=(const Impl &) = delete;

    void send(const Endpoint &peer, std::vector<std::uint8_t> bytes);
    std::vector<Datagram> takeIncoming();
    void checkHealth() const;

private:
    using Bytes = std::shared_ptr<std::vector<std::uint8_t>>;

    void runIo();
    void receive();
    void onReceive(const asio::error_code &error, std::size_t size);
    void store(std::size_t size);
    void startSend(asio::ip::udp::endpoint destination, Bytes data);
    void onSent(Bytes data, const asio::error_code &error, std::size_t size);
    void fail(const std::string &reason);

    asio::io_context m_io;
    asio::ip::udp::socket m_socket;
    std::array<std::uint8_t, UdpTransport::MAX_DATAGRAM_SIZE + 1> m_buffer;
    asio::ip::udp::endpoint m_sender;
    mutable std::mutex m_mutex;
    std::vector<Datagram> m_incoming;
    std::exception_ptr m_failure;
    std::atomic_size_t m_pending;
    std::thread m_worker;
};

} // namespace engine::net
