#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

#include "Packet.hpp"
#include "PendingMessage.hpp"

namespace engine::network {

class ReliableSender
{
public:
    using Time = std::chrono::steady_clock::time_point;

    static constexpr std::size_t MAX_PENDING = 64;
    static constexpr std::uint32_t ACK_WINDOW = 32;
    static constexpr std::chrono::milliseconds DEFAULT_ROUND_TRIP{100};
    static constexpr std::chrono::milliseconds MIN_RETRANSMIT_DELAY{100};
    static constexpr unsigned ROUND_TRIP_WEIGHT = 8;

    ReliableSender();

    std::uint16_t nextReliableId();
    void track(std::uint32_t sequence, Packet packet, Time now);
    void acknowledge(std::uint32_t ack, std::uint32_t ackBits, Time now);
    std::vector<Packet> expired(Time now);

    std::chrono::milliseconds roundTrip() const;
    std::size_t pending() const;

private:
    void confirm(std::uint32_t sequence, Time now);

    std::map<std::uint32_t, PendingMessage> m_pending;
    std::uint16_t m_nextReliableId;
    std::chrono::milliseconds m_roundTrip;
};

} // namespace engine::network
