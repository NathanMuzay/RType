#include "ReliableReceiver.hpp"

#include "Codec.hpp"

namespace engine::network {

ReliableReceiver::ReliableReceiver() : m_latest(), m_history(0), m_order(), m_seen()
{
}

void ReliableReceiver::receive(std::uint32_t sequence)
{
    if (!m_latest) {
        m_latest = sequence;
        m_history = 0;
        return;
    }
    if (Codec::isNewer(sequence, *m_latest)) {
        const std::uint32_t shift = sequence - *m_latest;
        if (shift > ACK_WINDOW)
            m_history = 0;
        else if (shift == ACK_WINDOW)
            m_history = 1u << (ACK_WINDOW - 1);
        else
            m_history = (m_history << shift) | (1u << (shift - 1));
        m_latest = sequence;
        return;
    }
    const std::uint32_t distance = *m_latest - sequence;
    if (distance >= 1 && distance <= ACK_WINDOW)
        m_history |= 1u << (distance - 1);
}

std::uint32_t ReliableReceiver::ack() const
{
    return m_latest.value_or(0);
}

std::uint32_t ReliableReceiver::ackBits() const
{
    return m_history;
}

bool ReliableReceiver::isDuplicate(std::uint16_t reliableId) const
{
    return m_seen.find(reliableId) != m_seen.end();
}

void ReliableReceiver::remember(std::uint16_t reliableId)
{
    if (!m_seen.insert(reliableId).second)
        return;
    m_order.push_back(reliableId);
    if (m_order.size() <= SEEN_HISTORY)
        return;
    m_seen.erase(m_order.front());
    m_order.pop_front();
}

} // namespace engine::network
