#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>

#include "SnapshotMessage.hpp"

namespace engine::network {

class SnapshotHistory
{
public:
    static constexpr std::size_t MAX_ENTRIES = 16;
    static constexpr std::uint32_t ACK_WINDOW = 32;

    SnapshotHistory();

    void remember(std::uint32_t sequence, SnapshotMessage snapshot);
    void acknowledge(std::uint32_t ack, std::uint32_t ackBits);
    const SnapshotMessage *baseline() const;
    std::optional<std::uint32_t> lastFull() const;
    void markFull(std::uint32_t tick);
    void forget();

private:
    void promote(std::uint32_t sequence);

    std::map<std::uint32_t, SnapshotMessage> m_sent;
    std::optional<SnapshotMessage> m_baseline;
    std::optional<std::uint32_t> m_lastFull;
};

} // namespace engine::network
