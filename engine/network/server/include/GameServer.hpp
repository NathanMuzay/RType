#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <random>
#include <vector>

#include "ClientSession.hpp"
#include "Codec.hpp"
#include "Datagram.hpp"
#include "Endpoint.hpp"
#include "GameRoom.hpp"
#include "ISessionFactory.hpp"
#include "NetworkSettings.hpp"
#include "Packet.hpp"
#include "UdpTransport.hpp"

namespace engine::network {

class GameServer
{
public:
    static constexpr unsigned MAX_CONNECT_REQUESTS_PER_WINDOW = 32;
    static constexpr std::chrono::seconds CONNECT_RATE_WINDOW{1};
    static constexpr std::chrono::milliseconds ACCEPT_REPEAT_DELAY{200};
    static constexpr std::chrono::seconds ROOM_UPDATE_INTERVAL{1};
    static constexpr std::uint32_t FULL_SNAPSHOT_SECONDS = 1;
    static constexpr std::uint32_t KEYFRAME_SECONDS = 5;

    GameServer(std::uint16_t port, const NetworkSettings &settings, ISessionFactory &factory);

    void poll();
    void tick(float dt);
    void shutdown();

private:
    using Time = ClientSession::Time;

    void receive(const net::Datagram &datagram, Time now);
    void handleConnect(const net::Datagram &datagram, const Packet &packet, Time now);
    void handleSessionPacket(std::size_t client, const Packet &packet, Time now);
    void handleDisconnect(std::size_t client, const Packet &packet);
    bool acceptSequence(ClientSession &session, const Packet &packet);
    void handleInput(std::size_t client, const InputMessage &input, Time now);
    void handleCreateRoom(std::size_t client, const CreateRoomMessage &request);
    void handleJoinRoom(std::size_t client, const JoinRoomMessage &request);
    void handleStartGame(std::size_t client);
    void handleKick(std::size_t client, const KickPlayerMessage &request);
    void handleConsole(std::size_t client, const ConsoleCommandMessage &request);
    void handleDeleteRoom(std::size_t client);
    void handleChat(std::size_t client, const ChatMessage &request, Time now);
    void confirm(std::size_t client);
    void remove(std::size_t client, std::uint8_t reason);

    void tickRoom(GameRoom &room, float dt, Time now);
    void seatClient(std::size_t client, GameRoom &room, std::size_t slot);
    void leaveRoom(std::size_t client, std::uint8_t reason = Protocol::LEFT_QUIT);
    void closeRoom(std::uint16_t roomId);
    void returnToRoom(GameRoom &room);
    GameRoom *roomOf(std::size_t client);
    std::optional<std::size_t> ownedRoomSlot(std::size_t client, GameRoom *room);
    std::uint16_t newRoomId();

    void sendRoomList(std::size_t client);
    void sendRoomJoined(std::size_t client, GameRoom &room);
    void sendRoomUpdate(GameRoom &room);
    void broadcastRoom(GameRoom &room, const Packet &packet);
    std::uint32_t sendTo(std::size_t client, Packet packet);
    void sendSnapshot(std::size_t client, const SnapshotMessage &snapshot);
    void send(const net::Endpoint &peer, Packet packet);
    void reject(const net::Endpoint &peer, std::uint8_t reason);
    std::optional<std::size_t> findClient(const net::Endpoint &peer) const;
    std::optional<std::size_t> findFreeClient() const;
    std::uint32_t newToken();
    static std::optional<std::uint32_t> tokenOf(const Packet &packet);

    NetworkSettings m_settings;
    Codec m_codec;
    net::UdpTransport m_transport;
    ISessionFactory &m_factory;
    std::vector<std::optional<ClientSession>> m_clients;
    std::map<std::uint16_t, GameRoom> m_rooms;
    std::mt19937 m_random;
    std::uint16_t m_nextRoomId;
    Time m_rateWindow;
    Time m_lastRoomUpdate;
    unsigned m_connectRequests;
};

} // namespace engine::network
