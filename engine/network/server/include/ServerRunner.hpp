#pragma once

#include <chrono>
#include <csignal>

#include "GameServer.hpp"

namespace engine::network {

class ServerRunner
{
public:
    static constexpr std::chrono::milliseconds IDLE_DELAY{1};

    static void run(GameServer &server, int tickRate);

private:
    static void stop(int signal);

    static volatile std::sig_atomic_t s_running;
};

} // namespace engine::network
