#include "DebugMetrics.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

#include "ProcessMemory.hpp"

namespace engine::client {

DebugMetrics::DebugMetrics()
    : m_since(std::chrono::steady_clock::now()), m_frames(0), m_lastIn(0), m_lastOut(0), m_pings()
{
}

void DebugMetrics::frame(
    ecs::UiStore &store, std::size_t entities, std::uint64_t bytesIn, std::uint64_t bytesOut, std::uint32_t pingMs)
{
    ++m_frames;
    const Time now = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed = now - m_since;
    if (elapsed < PUBLISH_INTERVAL)
        return;
    const double seconds = elapsed.count();
    store.setValue("debug.fps", std::to_string(std::lround(m_frames / seconds)));
    store.setValue("debug.ping", std::to_string(pingMs));
    store.setValue("debug.entities", std::to_string(entities));
    store.setValue("debug.memory", std::to_string(std::lround(ProcessMemory::residentBytes() / BYTES_PER_MEGABYTE)));
    const double receivedKb = static_cast<double>(bytesIn - m_lastIn) / BYTES_PER_KILOBYTE / seconds;
    const double sentKb = static_cast<double>(bytesOut - m_lastOut) / BYTES_PER_KILOBYTE / seconds;
    store.setValue("debug.in", oneDecimal(receivedKb));
    store.setValue("debug.out", oneDecimal(sentKb));
    m_pings.push_back(pingMs);
    if (m_pings.size() > HISTORY)
        m_pings.pop_front();
    for (std::size_t index = 0; index < HISTORY; ++index) {
        const std::size_t offset = HISTORY - m_pings.size();
        const std::uint32_t value = index < offset ? 0 : m_pings[index - offset];
        store.setValue("debug.ping" + std::to_string(index), std::to_string(value));
    }
    m_lastIn = bytesIn;
    m_lastOut = bytesOut;
    m_frames = 0;
    m_since = now;
}

std::string DebugMetrics::oneDecimal(double value)
{
    std::ostringstream text;
    text << std::fixed << std::setprecision(1) << value;
    return text.str();
}

void DebugMetrics::toggle(ecs::UiStore &store)
{
    store.setValue(VISIBLE_KEY, store.value(VISIBLE_KEY) == "yes" ? "no" : "yes");
}

} // namespace engine::client
