#pragma once

#include <chrono>

namespace engine {

class Clock
{
public:
    Clock();

    float elapsed() const;
    float restart();

private:
    std::chrono::steady_clock::time_point m_start;
};

} // namespace engine
