#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <unordered_set>

namespace engine::network {

class ReliableReceiver
{
public:
    static constexpr std::uint32_t ACK_WINDOW = 32;
    static constexpr std::size_t SEEN_HISTORY = 256;

    ReliableReceiver();

    void receive(std::uint32_t sequence);
    std::uint32_t ack() const;
    std::uint32_t ackBits() const;

    bool isDuplicate(std::uint16_t reliableId) const;
    void remember(std::uint16_t reliableId);

private:
    std::optional<std::uint32_t> m_latest;
    std::uint32_t m_history;
    std::deque<std::uint16_t> m_order;
    std::unordered_set<std::uint16_t> m_seen;
};

} // namespace engine::network
