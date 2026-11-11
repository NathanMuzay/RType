#include "ClientConnection.hpp"

#include <iostream>
#include <utility>

#include "Codec.hpp"
#include "NetError.hpp"

namespace engine::network {

ClientConnection::ClientConnection(net::Endpoint server, const NetworkSettings &settings, std::string name)
    : m_settings(settings), m_codec(settings), m_server(std::move(server)), m_name(std::move(name)), m_sequence(1),
      m_sender(), m_receiver(), m_token(0), m_attempts(0), m_lastConnect(), m_lastInput(), m_lastHeartbeat(),
      m_lastReceive(), m_lastTick(), m_serverSequence(), m_snapshot(), m_applied(), m_events(), m_bytesReceived(0),
      m_bytesSent(0), m_ping(0)
{
}

void ClientConnection::update(std::uint8_t buttons)
{
    const Time now = std::chrono::steady_clock::now();
    for (const net::Datagram &datagram : m_transport.poll()) {
        if (datagram.sender() != m_server)
            continue;
        m_bytesReceived += datagram.bytes().size();
        const auto packet = m_codec.decode(datagram.bytes(), Receiver::Client);
        if (!packet)
            continue;
        m_receiver.receive(packet->sequence());
        m_sender.acknowledge(packet->ack(), packet->ackBits(), now);
        if (packet->reliableId() != 0) {
            if (m_receiver.isDuplicate(packet->reliableId())) {
                m_lastReceive = now;
                continue;
            }
            m_receiver.remember(packet->reliableId());
        }
        if (handlePacket(*packet))
            m_lastReceive = now;
    }
    if (!connected()) {
        retryConnect(now);
        return;
    }
    if (now - m_lastReceive >= m_settings.sessionTimeout())
        throw net::NetError("ClientConnection: connection lost, the server stopped answering");
    for (Packet &packet : m_sender.expired(now))
        send(std::move(packet));
    sendPeriodic(buttons, now);
}

void ClientConnection::disconnect()
{
    if (!connected())
        return;
    send(Packet(DisconnectMessage(m_token)));
    m_token = 0;
}

std::vector<Packet> ClientConnection::takeEvents()
{
    std::vector<Packet> events;
    events.swap(m_events);
    return events;
}

std::optional<SnapshotMessage> ClientConnection::takeSnapshot()
{
    std::optional<SnapshotMessage> snapshot = std::move(m_snapshot);
    m_snapshot.reset();
    return snapshot;
}

bool ClientConnection::connected() const
{
    return m_token != 0;
}

bool ClientConnection::handlePacket(const Packet &packet)
{
    if (!connected())
        return handleConnectionReply(packet);
    if (packet.type() == MessageType::Accept || packet.type() == MessageType::Reject)
        return false;
    if (packet.holds<SnapshotMessage>()) {
        if (!handleSnapshot(packet.as<SnapshotMessage>()))
            return false;
    } else if (packet.holds<SnapshotDeltaMessage>()) {
        if (!handleDelta(packet.as<SnapshotDeltaMessage>()))
            return false;
    } else if (packet.reliableId() == 0 && m_serverSequence && !Codec::isNewer(packet.sequence(), *m_serverSequence)) {
        return false;
    }
    if (packet.type() == MessageType::ServerShutdown)
        throw net::NetError("ClientConnection: server stopped");
    if (packet.holds<HeartbeatAckMessage>())
        m_ping = nowMs(std::chrono::steady_clock::now()) - packet.as<HeartbeatAckMessage>().timestamp();
    if (packet.holds<RoomJoinedMessage>() || packet.holds<GameStartedMessage>()) {
        m_lastTick.reset();
        m_applied.clear();
    }
    if (!packet.holds<SnapshotMessage>() && !packet.holds<SnapshotDeltaMessage>()
        && !packet.holds<HeartbeatAckMessage>())
        m_events.push_back(packet);
    if (!m_serverSequence || Codec::isNewer(packet.sequence(), *m_serverSequence))
        m_serverSequence = packet.sequence();
    return true;
}

void ClientConnection::listRooms()
{
    sendIfConnected(Packet(ListRoomsMessage(m_token)));
}

void ClientConnection::createRoom(
    const std::string &name, const std::string &code, const std::vector<std::uint8_t> &rules)
{
    sendIfConnected(Packet(CreateRoomMessage(m_token, name, code, rules)));
}

void ClientConnection::joinRoom(std::uint16_t roomId, const std::string &code)
{
    sendIfConnected(Packet(JoinRoomMessage(m_token, roomId, code)));
}

void ClientConnection::sendChat(const std::string &text)
{
    if (connected() && !text.empty())
        send(Packet(ChatMessage(m_token, text)));
}

void ClientConnection::leaveRoom()
{
    sendIfConnected(Packet(LeaveRoomMessage(m_token)));
}

void ClientConnection::startGame()
{
    sendIfConnected(Packet(StartGameMessage(m_token)));
}

void ClientConnection::kickPlayer(std::uint8_t slot)
{
    sendIfConnected(Packet(KickPlayerMessage(m_token, slot)));
}

void ClientConnection::deleteRoom()
{
    sendIfConnected(Packet(DeleteRoomMessage(m_token)));
}

bool ClientConnection::handleSnapshot(const SnapshotMessage &snapshot)
{
    if (m_lastTick && !Codec::isNewer(snapshot.tick(), *m_lastTick))
        return false;
    m_lastTick = snapshot.tick();
    rememberSnapshot(snapshot);
    m_snapshot = snapshot;
    return true;
}

bool ClientConnection::handleDelta(const SnapshotDeltaMessage &delta)
{
    const auto base = m_applied.find(delta.baselineTick());
    if (base == m_applied.end())
        return false;
    return handleSnapshot(SnapshotDiff::apply(base->second, delta));
}

void ClientConnection::rememberSnapshot(const SnapshotMessage &snapshot)
{
    if (m_applied.size() >= APPLIED_HISTORY)
        m_applied.erase(m_applied.begin());
    m_applied.insert_or_assign(snapshot.tick(), snapshot);
}

bool ClientConnection::handleConnectionReply(const Packet &packet)
{
    if (packet.holds<RejectMessage>()) {
        if (packet.as<RejectMessage>().reason() == Protocol::REJECT_SERVER_FULL)
            throw net::NetError("ClientConnection: server full");
        throw net::NetError("ClientConnection: protocol version mismatch");
    }
    if (!packet.holds<AcceptMessage>() || m_attempts == 0)
        return false;
    const AcceptMessage &accept = packet.as<AcceptMessage>();
    m_token = accept.token();
    m_serverSequence = packet.sequence();
    std::cout << "Connected as player " << static_cast<int>(accept.playerId()) << std::endl;
    return true;
}

void ClientConnection::retryConnect(Time now)
{
    if (m_attempts != 0 && now - m_lastConnect < m_settings.connectRetryInterval())
        return;
    if (m_attempts >= m_settings.connectAttempts())
        throw net::NetError("ClientConnection: connection timed out, the server never answered");
    send(Packet(ConnectMessage(m_name)));
    ++m_attempts;
    m_lastConnect = now;
}

void ClientConnection::sendPeriodic(std::uint8_t buttons, Time now)
{
    if (now - m_lastInput >= m_settings.inputInterval()) {
        send(Packet(InputMessage(m_token, buttons)));
        m_lastInput = now;
    }
    if (now - m_lastHeartbeat >= m_settings.heartbeatInterval()) {
        send(Packet(HeartbeatMessage(m_token, nowMs(now))));
        m_lastHeartbeat = now;
    }
}

void ClientConnection::send(Packet packet)
{
    if (packet.reliableId() == 0 && Protocol::isReliable(packet.type()))
        packet.setReliableId(m_sender.nextReliableId());
    packet.setSequence(m_sequence++);
    packet.setAck(m_receiver.ack());
    packet.setAckBits(m_receiver.ackBits());
    if (packet.reliableId() != 0)
        m_sender.track(packet.sequence(), packet, std::chrono::steady_clock::now());
    const auto bytes = m_codec.encode(packet);
    m_bytesSent += bytes.size();
    m_transport.send(m_server, bytes);
}

std::uint64_t ClientConnection::bytesReceived() const
{
    return m_bytesReceived;
}

std::uint64_t ClientConnection::bytesSent() const
{
    return m_bytesSent;
}

std::uint32_t ClientConnection::pingMs() const
{
    return m_ping;
}

std::uint32_t ClientConnection::roundTripMs() const
{
    return static_cast<std::uint32_t>(m_sender.roundTrip().count());
}

std::uint32_t ClientConnection::nowMs(Time now)
{
    return static_cast<std::uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
}

void ClientConnection::sendCommand(const std::string &text)
{
    sendIfConnected(Packet(ConsoleCommandMessage(m_token, text)));
}

void ClientConnection::sendIfConnected(Packet packet)
{
    if (connected())
        send(std::move(packet));
}

} // namespace engine::network
