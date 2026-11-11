#include "ReliableSender.hpp"

#include <algorithm>
#include <utility>

namespace engine::network {

ReliableSender::ReliableSender() : m_pending(), m_nextReliableId(1), m_roundTrip(DEFAULT_ROUND_TRIP)
{
}

std::uint16_t ReliableSender::nextReliableId()
{
    if (m_nextReliableId == 0)
        m_nextReliableId = 1;
    return m_nextReliableId++;
}

void ReliableSender::track(std::uint32_t sequence, Packet packet, Time now)
{
    if (m_pending.size() >= MAX_PENDING)
        m_pending.erase(m_pending.begin());
    m_pending.emplace(sequence, PendingMessage(std::move(packet), now));
}

void ReliableSender::acknowledge(std::uint32_t ack, std::uint32_t ackBits, Time now)
{
    if (ack == 0)
        return;
    confirm(ack, now);
    for (std::uint32_t bit = 0; bit < ACK_WINDOW; ++bit) {
        if ((ackBits & (1u << bit)) != 0)
            confirm(ack - 1 - bit, now);
    }
}

std::vector<Packet> ReliableSender::expired(Time now)
{
    const std::chrono::milliseconds delay = std::max(MIN_RETRANSMIT_DELAY, m_roundTrip * 3 / 2);
    std::vector<Packet> packets;
    for (auto entry = m_pending.begin(); entry != m_pending.end();) {
        if (now - entry->second.sentAt() < delay) {
            ++entry;
            continue;
        }
        packets.push_back(entry->second.packet());
        entry = m_pending.erase(entry);
    }
    return packets;
}

std::chrono::milliseconds ReliableSender::roundTrip() const
{
    return m_roundTrip;
}

std::size_t ReliableSender::pending() const
{
    return m_pending.size();
}

void ReliableSender::confirm(std::uint32_t sequence, Time now)
{
    const auto entry = m_pending.find(sequence);
    if (entry == m_pending.end())
        return;
    const auto sample = std::chrono::duration_cast<std::chrono::milliseconds>(now - entry->second.sentAt());
    m_roundTrip = (m_roundTrip * (ROUND_TRIP_WEIGHT - 1) + sample) / ROUND_TRIP_WEIGHT;
    m_pending.erase(entry);
}

} // namespace engine::network
