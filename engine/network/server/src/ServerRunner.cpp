#include "ServerRunner.hpp"

#include <thread>

#include "Clock.hpp"
#include "FixedTimestep.hpp"

namespace engine::network {

volatile std::sig_atomic_t ServerRunner::s_running = 0;

void ServerRunner::run(GameServer &server, int tickRate)
{
    s_running = 1;
    std::signal(SIGINT, &ServerRunner::stop);
    std::signal(SIGTERM, &ServerRunner::stop);
    Clock clock;
    FixedTimestep timestep(1.f / static_cast<float>(tickRate));
    while (s_running) {
        server.poll();
        timestep.addTime(clock.restart());
        while (timestep.step())
            server.tick(timestep.stepSeconds());
        std::this_thread::sleep_for(IDLE_DELAY);
    }
    server.shutdown();
}

void ServerRunner::stop(int)
{
    s_running = 0;
}

} // namespace engine::network
