#include "SnapshotHistory.hpp"

#include <utility>

#include "Codec.hpp"

namespace engine::network {

SnapshotHistory::SnapshotHistory() : m_sent(), m_baseline(), m_lastFull()
{
}

void SnapshotHistory::remember(std::uint32_t sequence, SnapshotMessage snapshot)
{
    if (sequence == 0)
        return;
    if (m_sent.size() >= MAX_ENTRIES)
        m_sent.erase(m_sent.begin());
    m_sent.insert_or_assign(sequence, std::move(snapshot));
}

void SnapshotHistory::acknowledge(std::uint32_t ack, std::uint32_t ackBits)
{
    if (ack == 0)
        return;
    promote(ack);
    for (std::uint32_t bit = 0; bit < ACK_WINDOW; ++bit) {
        if ((ackBits & (1u << bit)) != 0)
            promote(ack - 1 - bit);
    }
}

const SnapshotMessage *SnapshotHistory::baseline() const
{
    return m_baseline ? &*m_baseline : nullptr;
}

std::optional<std::uint32_t> SnapshotHistory::lastFull() const
{
    return m_lastFull;
}

void SnapshotHistory::markFull(std::uint32_t tick)
{
    m_lastFull = tick;
}

void SnapshotHistory::forget()
{
    m_sent.clear();
    m_baseline.reset();
    m_lastFull.reset();
}

void SnapshotHistory::promote(std::uint32_t sequence)
{
    const auto entry = m_sent.find(sequence);
    if (entry == m_sent.end())
        return;
    if (!m_baseline || Codec::isNewer(entry->second.tick(), m_baseline->tick()))
        m_baseline = entry->second;
    m_sent.erase(entry);
}

} // namespace engine::network
