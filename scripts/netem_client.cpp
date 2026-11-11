// Headless integration probe using the production ClientConnection, codec and transport.
#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>
#include <nlohmann/json.hpp>
#include "ClientConnection.hpp"
#include "DataNode.hpp"

using namespace engine::network;
using Clock = std::chrono::steady_clock;
using Json = nlohmann::json;

struct Player {
    std::unique_ptr<ClientConnection> connection;
    bool joined = false;
    bool started = false;
    unsigned joinedEvents = 0, startedEvents = 0, snapshots = 0, backwards = 0;
    std::uint16_t room = 0;
    std::optional<std::uint32_t> firstTick, lastTick;
    double firstAt = 0, lastAt = 0, maxGap = 0;
    std::uint64_t rxStart = 0, txStart = 0;
    std::vector<unsigned> ping;
};

int main(int argc, char **argv)
{
    Json result;
    try {
        if (argc != 4)
            throw std::runtime_error("usage: netem_client game.json port seconds");
        const auto settings = NetworkSettings::fromGame(engine::scene::DataNode::fromFile(argv[1]));
        const auto port = static_cast<std::uint16_t>(std::stoi(argv[2]));
        const double duration = std::stod(argv[3]);
        Player players[2];
        for (std::size_t index = 0; index < 2; ++index)
            players[index].connection = std::make_unique<ClientConnection>(
                engine::net::Endpoint("127.0.0.1", port), settings, "probe" + std::to_string(index + 1));
        const auto origin = Clock::now();
        auto elapsed = [&] { return std::chrono::duration<double>(Clock::now() - origin).count(); };
        bool created = false, joinSent = false, startSent = false;
        auto pump = [&](bool measuring, double since) {
            for (auto &p : players) {
                p.connection->update(measuring ? Protocol::BUTTON_FIRE : 0);
                for (const auto &event : p.connection->takeEvents()) {
                    if (event.holds<RoomJoinedMessage>()) {
                        p.joined = true;
                        p.room = event.as<RoomJoinedMessage>().roomId();
                        ++p.joinedEvents;
                    }
                    if (event.holds<GameStartedMessage>()) {
                        p.started = true;
                        ++p.startedEvents;
                    }
                    if (event.holds<RoomRejectedMessage>())
                        throw std::runtime_error("room request rejected");
                }
                if (auto snapshot = p.connection->takeSnapshot(); snapshot && measuring) {
                    if (p.lastTick && snapshot->tick() <= *p.lastTick)
                        ++p.backwards;
                    if (!p.firstTick) {
                        p.firstTick = snapshot->tick();
                        p.firstAt = since;
                        p.maxGap = since;
                    } else {
                        p.maxGap = std::max(p.maxGap, since - p.lastAt);
                    }
                    p.lastTick = snapshot->tick();
                    p.lastAt = since;
                    ++p.snapshots;
                }
            }
        };
        while (!(players[0].started && players[1].started)) {
            pump(false, 0);
            if (elapsed() > 18)
                throw std::runtime_error("setup timeout (connect/create/join/start)");
            if (!created && players[0].connection->connected() && players[1].connection->connected()) {
                players[0].connection->createRoom("netem", "", std::vector<std::uint8_t>(settings.ruleCount(), 0));
                created = true;
            }
            if (!joinSent && players[0].joined) {
                players[1].connection->joinRoom(players[0].room, "");
                joinSent = true;
            }
            if (!startSent && players[1].joined) {
                players[0].connection->startGame();
                startSent = true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
        result["setup_seconds"] = elapsed();
        for (auto &p : players) {
            p.rxStart = p.connection->bytesReceived();
            p.txStart = p.connection->bytesSent();
        }
        std::cout << "NETEM_READY" << std::endl;
        const auto begin = Clock::now();
        double lastSample = 0;
        double seconds = 0;
        while ((seconds = std::chrono::duration<double>(Clock::now() - begin).count()) < duration) {
            pump(true, seconds);
            if (seconds - lastSample >= 1) {
                for (auto &p : players)
                    p.ping.push_back(p.connection->pingMs());
                lastSample = seconds;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
        bool passed = true;
        result["measurement_seconds"] = seconds;
        for (auto &p : players) {
            std::sort(p.ping.begin(), p.ping.end());
            const double observed = p.lastAt - p.firstAt;
            p.maxGap = std::max(p.maxGap, seconds - p.lastAt);
            const bool ok = p.snapshots >= 10 && p.backwards == 0
                && p.joinedEvents == 1 && p.startedEvents == 1 && p.maxGap < 5;
            passed = passed && ok;
            result["clients"].push_back({
                {"passed", ok}, {"snapshots", p.snapshots},
                {"snapshots_per_second", p.snapshots / seconds},
                {"simulation_ticks_per_second", observed > 0 ? (*p.lastTick - *p.firstTick) / observed : 0},
                {"max_snapshot_gap_seconds", p.maxGap}, {"backwards_snapshots", p.backwards},
                {"room_joined_events", p.joinedEvents}, {"game_started_events", p.startedEvents},
                {"rx_bytes_per_second", (p.connection->bytesReceived() - p.rxStart) / seconds},
                {"tx_bytes_per_second", (p.connection->bytesSent() - p.txStart) / seconds},
                {"ping_median_ms", p.ping.empty() ? 0 : p.ping[p.ping.size() / 2]},
                {"ping_max_ms", p.ping.empty() ? 0 : p.ping.back()},
                {"ping_samples_ms", p.ping}
            });
            p.connection->disconnect();
        }
        result["passed"] = passed;
    } catch (const std::exception &error) {
        result["passed"] = false;
        result["error"] = error.what();
    }
    std::cout << "NETEM_RESULT " << result.dump() << std::endl;
    return result.value("passed", false) ? 0 : 1;
}
