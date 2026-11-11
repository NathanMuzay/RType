#pragma once

#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "Codec.hpp"
#include "Endpoint.hpp"
#include "NetworkSettings.hpp"
#include "Packet.hpp"
#include "ReliableReceiver.hpp"
#include "ReliableSender.hpp"
#include "SnapshotDeltaMessage.hpp"
#include "SnapshotDiff.hpp"
#include "SnapshotMessage.hpp"
#include "UdpTransport.hpp"

namespace engine::network {

class ClientConnection
{
public:
    static constexpr std::size_t APPLIED_HISTORY = 32;

    ClientConnection(net::Endpoint server, const NetworkSettings &settings, std::string name);

    void update(std::uint8_t buttons);
    void disconnect();
    bool connected() const;
    std::optional<SnapshotMessage> takeSnapshot();
    std::vector<Packet> takeEvents();

    void listRooms();
    void createRoom(const std::string &name, const std::string &code, const std::vector<std::uint8_t> &rules);
    void joinRoom(std::uint16_t roomId, const std::string &code);
    void leaveRoom();
    void startGame();
    void kickPlayer(std::uint8_t slot);
    void deleteRoom();
    void sendCommand(const std::string &text);
    void sendChat(const std::string &text);

    std::uint64_t bytesReceived() const;
    std::uint64_t bytesSent() const;
    std::uint32_t pingMs() const;
    std::uint32_t roundTripMs() const;

private:
    using Time = std::chrono::steady_clock::time_point;

    bool handlePacket(const Packet &packet);
    bool handleSnapshot(const SnapshotMessage &snapshot);
    bool handleDelta(const SnapshotDeltaMessage &delta);
    void rememberSnapshot(const SnapshotMessage &snapshot);
    bool handleConnectionReply(const Packet &packet);
    void retryConnect(Time now);
    void sendPeriodic(std::uint8_t buttons, Time now);
    void send(Packet packet);
    void sendIfConnected(Packet packet);
    static std::uint32_t nowMs(Time now);

    NetworkSettings m_settings;
    Codec m_codec;
    net::UdpTransport m_transport;
    net::Endpoint m_server;
    std::string m_name;
    std::uint32_t m_sequence;
    ReliableSender m_sender;
    ReliableReceiver m_receiver;
    std::uint32_t m_token;
    unsigned m_attempts;
    Time m_lastConnect;
    Time m_lastInput;
    Time m_lastHeartbeat;
    Time m_lastReceive;
    std::optional<std::uint32_t> m_lastTick;
    std::optional<std::uint32_t> m_serverSequence;
    std::optional<SnapshotMessage> m_snapshot;
    std::map<std::uint32_t, SnapshotMessage> m_applied;
    std::vector<Packet> m_events;
    std::uint64_t m_bytesReceived;
    std::uint64_t m_bytesSent;
    std::uint32_t m_ping;
};

} // namespace engine::network
