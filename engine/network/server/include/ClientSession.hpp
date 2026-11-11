#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

#include "Endpoint.hpp"
#include "ReliableReceiver.hpp"
#include "ReliableSender.hpp"
#include "SnapshotHistory.hpp"

namespace engine::network {

class ClientSession
{
public:
    using Time = std::chrono::steady_clock::time_point;

    ClientSession(net::Endpoint peer, std::uint32_t token, Time now);

    const net::Endpoint &peer() const;
    std::uint32_t token() const;
    const std::string &name() const;
    void setName(std::string name);
    bool confirmed() const;
    void confirm();

    Time lastSeen() const;
    void markSeen(Time now);
    Time lastInput() const;
    void markInput(Time now);
    Time lastAccept() const;
    void markAccepted(Time now);
    Time lastChat() const;
    void markChat(Time now);

    std::optional<std::uint32_t> inputSequence() const;
    void setInputSequence(std::uint32_t sequence);
    std::optional<std::uint32_t> heartbeatSequence() const;
    void setHeartbeatSequence(std::uint32_t sequence);
    std::uint32_t nextOutgoingSequence();
    ReliableSender &sender();
    ReliableReceiver &receiver();
    SnapshotHistory &snapshots();

    std::optional<std::uint16_t> roomId() const;
    void enterRoom(std::uint16_t roomId);
    void leaveRoom();
    unsigned failedJoins() const;
    void addFailedJoin();

private:
    net::Endpoint m_peer;
    std::uint32_t m_token;
    std::string m_name;
    bool m_confirmed;
    Time m_lastSeen;
    Time m_lastInput;
    Time m_lastAccept;
    Time m_lastChat;
    std::optional<std::uint32_t> m_inputSequence;
    std::optional<std::uint32_t> m_heartbeatSequence;
    std::uint32_t m_outgoingSequence;
    ReliableSender m_sender;
    ReliableReceiver m_receiver;
    SnapshotHistory m_snapshots;
    std::optional<std::uint16_t> m_roomId;
    unsigned m_failedJoins;
};

} // namespace engine::network
