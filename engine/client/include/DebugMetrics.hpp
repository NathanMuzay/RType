#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>

#include "UiStore.hpp"

namespace engine::client {

class DebugMetrics
{
public:
    static constexpr std::chrono::milliseconds PUBLISH_INTERVAL{500};
    static constexpr std::size_t HISTORY = 30;
    static constexpr double BYTES_PER_KILOBYTE = 1024.0;
    static constexpr double BYTES_PER_MEGABYTE = 1024.0 * 1024.0;
    static constexpr const char *VISIBLE_KEY = "debug.visible";

    DebugMetrics();

    void frame(
        ecs::UiStore &store, std::size_t entities, std::uint64_t bytesIn, std::uint64_t bytesOut, std::uint32_t pingMs);
    static void toggle(ecs::UiStore &store);

private:
    static std::string oneDecimal(double value);

    using Time = std::chrono::steady_clock::time_point;

    Time m_since;
    unsigned int m_frames;
    std::uint64_t m_lastIn;
    std::uint64_t m_lastOut;
    std::deque<std::uint32_t> m_pings;
};

} // namespace engine::client
