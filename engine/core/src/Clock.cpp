#include "Clock.hpp"

namespace engine {

Clock::Clock() : m_start(std::chrono::steady_clock::now())
{
}

float Clock::elapsed() const
{
    return std::chrono::duration<float>(std::chrono::steady_clock::now() - m_start).count();
}

float Clock::restart()
{
    auto now = std::chrono::steady_clock::now();
    float seconds = std::chrono::duration<float>(now - m_start).count();
    m_start = now;
    return seconds;
}

} // namespace engine
