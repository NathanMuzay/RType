#pragma once

#include <cstdint>
#include <variant>

#include "AcceptMessage.hpp"
#include "ChatBroadcastMessage.hpp"
#include "ChatMessage.hpp"
#include "ConnectMessage.hpp"
#include "ConsoleCommandMessage.hpp"
#include "ConsoleTextMessage.hpp"
#include "CreateRoomMessage.hpp"
#include "DeleteRoomMessage.hpp"
#include "DisconnectMessage.hpp"
#include "EntityDestroyedMessage.hpp"
#include "GameStartedMessage.hpp"
#include "HeartbeatAckMessage.hpp"
#include "HeartbeatMessage.hpp"
#include "InputMessage.hpp"
#include "JoinRoomMessage.hpp"
#include "KickedMessage.hpp"
#include "KickPlayerMessage.hpp"
#include "LeaveRoomMessage.hpp"
#include "ListRoomsMessage.hpp"
#include "PlayerJoinedMessage.hpp"
#include "PlayerKilledMessage.hpp"
#include "PlayerLeftMessage.hpp"
#include "PlaySoundMessage.hpp"
#include "Protocol.hpp"
#include "RejectMessage.hpp"
#include "RoomClosedMessage.hpp"
#include "RoomJoinedMessage.hpp"
#include "RoomListMessage.hpp"
#include "RoomRejectedMessage.hpp"
#include "RoomUpdateMessage.hpp"
#include "ServerShutdownMessage.hpp"
#include "SnapshotDeltaMessage.hpp"
#include "SnapshotMessage.hpp"
#include "StartGameMessage.hpp"

namespace engine::network {

class Packet
{
public:
    using Body = std::variant<ConnectMessage, InputMessage, HeartbeatMessage, DisconnectMessage, AcceptMessage,
        RejectMessage, SnapshotMessage, HeartbeatAckMessage, EntityDestroyedMessage, PlayerJoinedMessage,
        PlayerKilledMessage, PlayerLeftMessage, ServerShutdownMessage, ListRoomsMessage, CreateRoomMessage,
        JoinRoomMessage, LeaveRoomMessage, StartGameMessage, KickPlayerMessage, DeleteRoomMessage, RoomListMessage,
        RoomJoinedMessage, RoomRejectedMessage, RoomUpdateMessage, GameStartedMessage, KickedMessage, RoomClosedMessage,
        PlaySoundMessage, ConsoleCommandMessage, ConsoleTextMessage, SnapshotDeltaMessage, ChatMessage,
        ChatBroadcastMessage>;

    template <class Message>
    explicit Packet(Message message, std::uint32_t sequence = 0, std::uint8_t version = Protocol::VERSION);

    MessageType type() const;
    std::uint8_t version() const;
    std::uint32_t sequence() const;
    void setSequence(std::uint32_t sequence);
    std::uint32_t ack() const;
    void setAck(std::uint32_t ack);
    std::uint32_t ackBits() const;
    void setAckBits(std::uint32_t ackBits);
    std::uint16_t reliableId() const;
    void setReliableId(std::uint16_t reliableId);

    template <class Message>
    bool holds() const;
    template <class Message>
    const Message &as() const;
    template <class Visitor>
    decltype(auto) visit(const Visitor &visitor) const;

private:
    MessageType m_type;
    std::uint8_t m_version;
    std::uint32_t m_sequence;
    std::uint32_t m_ack;
    std::uint32_t m_ackBits;
    std::uint16_t m_reliableId;
    Body m_body;
};

} // namespace engine::network

#include "Packet.tpp"
