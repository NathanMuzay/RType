#include "ClientSession.hpp"

#include <utility>

namespace engine::network {

ClientSession::ClientSession(net::Endpoint peer, std::uint32_t token, Time now)
    : m_peer(std::move(peer)), m_token(token), m_name(), m_confirmed(false), m_lastSeen(now), m_lastInput(now),
      m_lastAccept(), m_lastChat(), m_inputSequence(), m_heartbeatSequence(), m_outgoingSequence(1), m_sender(),
      m_receiver(), m_snapshots(), m_roomId(), m_failedJoins(0)
{
}

std::optional<std::uint16_t> ClientSession::roomId() const
{
    return m_roomId;
}

void ClientSession::enterRoom(std::uint16_t roomId)
{
    m_roomId = roomId;
}

void ClientSession::leaveRoom()
{
    m_roomId.reset();
}

unsigned ClientSession::failedJoins() const
{
    return m_failedJoins;
}

void ClientSession::addFailedJoin()
{
    ++m_failedJoins;
}

const net::Endpoint &ClientSession::peer() const
{
    return m_peer;
}

std::uint32_t ClientSession::token() const
{
    return m_token;
}

const std::string &ClientSession::name() const
{
    return m_name;
}

void ClientSession::setName(std::string name)
{
    m_name = std::move(name);
}

bool ClientSession::confirmed() const
{
    return m_confirmed;
}

void ClientSession::confirm()
{
    m_confirmed = true;
}

ClientSession::Time ClientSession::lastSeen() const
{
    return m_lastSeen;
}

void ClientSession::markSeen(Time now)
{
    m_lastSeen = now;
}

ClientSession::Time ClientSession::lastInput() const
{
    return m_lastInput;
}

void ClientSession::markInput(Time now)
{
    m_lastInput = now;
}

ClientSession::Time ClientSession::lastAccept() const
{
    return m_lastAccept;
}

void ClientSession::markAccepted(Time now)
{
    m_lastAccept = now;
}

ClientSession::Time ClientSession::lastChat() const
{
    return m_lastChat;
}

void ClientSession::markChat(Time now)
{
    m_lastChat = now;
}

std::optional<std::uint32_t> ClientSession::inputSequence() const
{
    return m_inputSequence;
}

void ClientSession::setInputSequence(std::uint32_t sequence)
{
    m_inputSequence = sequence;
}

std::optional<std::uint32_t> ClientSession::heartbeatSequence() const
{
    return m_heartbeatSequence;
}

void ClientSession::setHeartbeatSequence(std::uint32_t sequence)
{
    m_heartbeatSequence = sequence;
}

std::uint32_t ClientSession::nextOutgoingSequence()
{
    return m_outgoingSequence++;
}

ReliableSender &ClientSession::sender()
{
    return m_sender;
}

ReliableReceiver &ClientSession::receiver()
{
    return m_receiver;
}

SnapshotHistory &ClientSession::snapshots()
{
    return m_snapshots;
}

} // namespace engine::network
