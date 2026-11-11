#include "GameServer.hpp"

#include "PacketVisitors.hpp"
#include "SnapshotDiff.hpp"

#include <iostream>
#include <utility>

namespace engine::network {

GameServer::GameServer(std::uint16_t port, const NetworkSettings &settings, ISessionFactory &factory)
    : m_settings(settings), m_codec(settings), m_transport(port), m_factory(factory), m_clients(settings.maxClients()),
      m_rooms(), m_random(std::random_device{}()), m_nextRoomId(1), m_rateWindow(), m_lastRoomUpdate(),
      m_connectRequests(0)
{
    std::cout << "Server listening on UDP port " << port << std::endl;
    if (const auto address = net::UdpTransport::primaryLocalAddress())
        std::cout << "Players on your network can join with: -h " << *address << " -p " << port << std::endl;
}

void GameServer::poll()
{
    const Time now = std::chrono::steady_clock::now();
    for (std::size_t client = 0; client < m_clients.size(); ++client) {
        if (m_clients[client] && now - m_clients[client]->lastSeen() >= m_settings.sessionTimeout())
            remove(client, Protocol::LEFT_TIMEOUT);
    }
    for (const net::Datagram &datagram : m_transport.poll())
        receive(datagram, now);
    for (std::size_t client = 0; client < m_clients.size(); ++client) {
        if (!m_clients[client])
            continue;
        for (Packet &packet : m_clients[client]->sender().expired(now))
            sendTo(client, std::move(packet));
    }
}

void GameServer::tick(float dt)
{
    const Time now = std::chrono::steady_clock::now();
    for (auto &entry : m_rooms) {
        if (entry.second.playing())
            tickRoom(entry.second, dt, now);
    }
    if (now - m_lastRoomUpdate < ROOM_UPDATE_INTERVAL)
        return;
    m_lastRoomUpdate = now;
    for (auto &entry : m_rooms)
        sendRoomUpdate(entry.second);
}

void GameServer::shutdown()
{
    for (std::size_t client = 0; client < m_clients.size(); ++client) {
        if (m_clients[client] && m_clients[client]->confirmed())
            sendTo(client, Packet(ServerShutdownMessage()));
    }
}

void GameServer::receive(const net::Datagram &datagram, Time now)
{
    const auto packet = m_codec.decode(datagram.bytes(), Receiver::Server);
    if (!packet)
        return;
    if (packet->type() == MessageType::Connect) {
        handleConnect(datagram, *packet, now);
        return;
    }
    const auto client = findClient(datagram.sender());
    if (!client || tokenOf(*packet) != m_clients[*client]->token())
        return;
    ClientSession &session = *m_clients[*client];
    session.receiver().receive(packet->sequence());
    session.sender().acknowledge(packet->ack(), packet->ackBits(), now);
    session.snapshots().acknowledge(packet->ack(), packet->ackBits());
    if (packet->reliableId() != 0) {
        if (session.receiver().isDuplicate(packet->reliableId())) {
            session.markSeen(now);
            return;
        }
        session.receiver().remember(packet->reliableId());
    }
    handleSessionPacket(*client, *packet, now);
}

void GameServer::handleConnect(const net::Datagram &datagram, const Packet &packet, Time now)
{
    if (now - m_rateWindow >= CONNECT_RATE_WINDOW) {
        m_rateWindow = now;
        m_connectRequests = 0;
    }
    if (++m_connectRequests > MAX_CONNECT_REQUESTS_PER_WINDOW)
        return;
    if (packet.version() != Protocol::VERSION) {
        reject(datagram.sender(), Protocol::REJECT_VERSION_MISMATCH);
        return;
    }
    std::optional<std::size_t> client = findClient(datagram.sender());
    const bool known = client.has_value();
    if (!known) {
        client = findFreeClient();
        if (!client) {
            reject(datagram.sender(), Protocol::REJECT_SERVER_FULL);
            return;
        }
        m_clients[*client].emplace(datagram.sender(), newToken(), now);
    }
    if (!packet.as<ConnectMessage>().name().empty())
        m_clients[*client]->setName(packet.as<ConnectMessage>().name());
    m_clients[*client]->receiver().receive(packet.sequence());
    if (known && now - m_clients[*client]->lastAccept() < ACCEPT_REPEAT_DELAY)
        return;
    m_clients[*client]->markAccepted(now);
    sendTo(*client, Packet(AcceptMessage(m_clients[*client]->token(), static_cast<std::uint8_t>(*client))));
}

void GameServer::handleSessionPacket(std::size_t client, const Packet &packet, Time now)
{
    if (packet.type() == MessageType::Disconnect) {
        handleDisconnect(client, packet);
        return;
    }
    ClientSession &session = *m_clients[client];
    const bool sequenced = packet.holds<InputMessage>() || packet.holds<HeartbeatMessage>();
    if (sequenced && !acceptSequence(session, packet))
        return;
    session.markSeen(now);
    if (!session.confirmed())
        confirm(client);
    if (packet.holds<InputMessage>())
        handleInput(client, packet.as<InputMessage>(), now);
    else if (packet.holds<HeartbeatMessage>())
        sendTo(client, Packet(HeartbeatAckMessage(packet.as<HeartbeatMessage>().timestamp())));
    else if (packet.holds<ListRoomsMessage>())
        sendRoomList(client);
    else if (packet.holds<CreateRoomMessage>())
        handleCreateRoom(client, packet.as<CreateRoomMessage>());
    else if (packet.holds<JoinRoomMessage>())
        handleJoinRoom(client, packet.as<JoinRoomMessage>());
    else if (packet.holds<LeaveRoomMessage>())
        leaveRoom(client);
    else if (packet.holds<StartGameMessage>())
        handleStartGame(client);
    else if (packet.holds<KickPlayerMessage>())
        handleKick(client, packet.as<KickPlayerMessage>());
    else if (packet.holds<DeleteRoomMessage>())
        handleDeleteRoom(client);
    else if (packet.holds<ConsoleCommandMessage>())
        handleConsole(client, packet.as<ConsoleCommandMessage>());
    else if (packet.holds<ChatMessage>())
        handleChat(client, packet.as<ChatMessage>(), now);
}

void GameServer::handleDisconnect(std::size_t client, const Packet &packet)
{
    const ClientSession &session = *m_clients[client];
    const auto input = session.inputSequence();
    const auto heartbeat = session.heartbeatSequence();
    const bool staleInput = input && !Codec::isNewer(packet.sequence(), *input);
    const bool staleHeartbeat = heartbeat && !Codec::isNewer(packet.sequence(), *heartbeat);
    if (!staleInput && !staleHeartbeat)
        remove(client, Protocol::LEFT_QUIT);
}

bool GameServer::acceptSequence(ClientSession &session, const Packet &packet)
{
    const bool input = packet.holds<InputMessage>();
    const auto previous = input ? session.inputSequence() : session.heartbeatSequence();
    if (previous && !Codec::isNewer(packet.sequence(), *previous))
        return false;
    if (input)
        session.setInputSequence(packet.sequence());
    else
        session.setHeartbeatSequence(packet.sequence());
    return true;
}

void GameServer::handleInput(std::size_t client, const InputMessage &input, Time now)
{
    m_clients[client]->markInput(now);
    GameRoom *room = roomOf(client);
    if (!room || !room->playing())
        return;
    if (const auto slot = room->slotOf(client))
        room->session().setInput(*slot, input.buttons());
}

void GameServer::confirm(std::size_t client)
{
    m_clients[client]->confirm();
    std::cout << "Client " << client << " connected" << std::endl;
}

void GameServer::remove(std::size_t client, std::uint8_t reason)
{
    leaveRoom(client, reason);
    const bool confirmed = m_clients[client]->confirmed();
    m_clients[client].reset();
    if (confirmed)
        std::cout << "Client " << client << " left (reason " << static_cast<int>(reason) << ")" << std::endl;
}

void GameServer::tickRoom(GameRoom &room, float dt, Time now)
{
    for (std::size_t slot = 0; slot < room.capacity(); ++slot) {
        const auto client = room.clientAt(slot);
        if (client && now - m_clients[*client]->lastInput() >= m_settings.inputReleaseDelay())
            room.session().setInput(slot, 0);
    }
    room.session().update(dt);
    for (const Packet &event : room.session().takeEvents())
        broadcastRoom(room, event);
    if (room.session().tick() % m_settings.snapshotEveryTicks() == 0) {
        const SnapshotMessage snapshot = room.session().snapshot();
        for (std::size_t client : room.clients())
            sendSnapshot(client, snapshot);
    }
    if (room.session().finished())
        returnToRoom(room);
}

void GameServer::returnToRoom(GameRoom &room)
{
    room.setPlaying(false);
    room.resetSession(m_factory.createSession(room.rules()));
    for (std::size_t client : room.clients()) {
        m_clients[client]->snapshots().forget();
        sendRoomJoined(client, room);
    }
    sendRoomUpdate(room);
    std::cout << "Room " << room.id() << " finished its game" << std::endl;
}

std::uint32_t GameServer::sendTo(std::size_t client, Packet packet)
{
    if (!m_clients[client])
        return 0;
    ClientSession &session = *m_clients[client];
    if (packet.reliableId() == 0 && Protocol::isReliable(packet.type()))
        packet.setReliableId(session.sender().nextReliableId());
    packet.setSequence(session.nextOutgoingSequence());
    packet.setAck(session.receiver().ack());
    packet.setAckBits(session.receiver().ackBits());
    if (packet.reliableId() != 0)
        session.sender().track(packet.sequence(), packet, std::chrono::steady_clock::now());
    m_transport.send(session.peer(), m_codec.encode(packet));
    return packet.sequence();
}

void GameServer::sendSnapshot(std::size_t client, const SnapshotMessage &snapshot)
{
    SnapshotHistory &history = m_clients[client]->snapshots();
    const SnapshotMessage *base = history.baseline();
    const std::uint32_t staleAfter = FULL_SNAPSHOT_SECONDS * static_cast<std::uint32_t>(m_settings.tickRate());
    const bool fresh =
        base != nullptr && Codec::isNewer(snapshot.tick(), base->tick()) && snapshot.tick() - base->tick() < staleAfter;
    const std::optional<std::uint32_t> lastFull = history.lastFull();
    const std::uint32_t keyframeAfter = KEYFRAME_SECONDS * static_cast<std::uint32_t>(m_settings.tickRate());
    const bool overdue = !lastFull || snapshot.tick() - *lastFull >= keyframeAfter;
    const bool differential = fresh && base->tick() != 0 && !overdue;
    if (!differential)
        history.markFull(snapshot.tick());
    const std::uint32_t sequence =
        differential ? sendTo(client, Packet(SnapshotDiff::build(*base, snapshot))) : sendTo(client, Packet(snapshot));
    if (sequence != 0)
        history.remember(sequence, snapshot);
}

void GameServer::send(const net::Endpoint &peer, Packet packet)
{
    m_transport.send(peer, m_codec.encode(packet));
}

void GameServer::reject(const net::Endpoint &peer, std::uint8_t reason)
{
    send(peer, Packet(RejectMessage(reason)));
}

std::optional<std::size_t> GameServer::findClient(const net::Endpoint &peer) const
{
    for (std::size_t client = 0; client < m_clients.size(); ++client) {
        if (m_clients[client] && m_clients[client]->peer() == peer)
            return client;
    }
    return std::nullopt;
}

std::optional<std::size_t> GameServer::findFreeClient() const
{
    for (std::size_t client = 0; client < m_clients.size(); ++client) {
        if (!m_clients[client])
            return client;
    }
    return std::nullopt;
}

std::uint32_t GameServer::newToken()
{
    while (true) {
        const auto token = static_cast<std::uint32_t>(m_random());
        bool used = token == 0;
        for (const auto &client : m_clients) {
            if (client && client->token() == token)
                used = true;
        }
        if (!used)
            return token;
    }
}

std::optional<std::uint32_t> GameServer::tokenOf(const Packet &packet)
{
    return packet.visit(TokenReader());
}

} // namespace engine::network
