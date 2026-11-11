#include "Codec.hpp"

#include "PacketVisitors.hpp"

#include <utility>

#include "NetError.hpp"

namespace engine::network {

Codec::Codec(NetworkSettings settings) : m_settings(std::move(settings))
{
}

bool Codec::isNewer(std::uint32_t candidate, std::uint32_t previous)
{
    const std::uint32_t distance = candidate - previous;
    return distance != 0 && distance < HALF_SEQUENCE_RANGE;
}

std::vector<std::uint8_t> Codec::encode(const Packet &packet) const
{
    if (!isValid(packet))
        throw net::NetError("Codec: invalid outgoing packet");
    net::ByteWriter writer(Protocol::MAX_PACKET_BYTES);
    writer.writeU16(Protocol::MAGIC);
    writer.writeU8(packet.version());
    writer.writeU8(static_cast<std::uint8_t>(packet.type()));
    writer.writeU32(packet.sequence());
    writer.writeU32(packet.ack());
    writer.writeU32(packet.ackBits());
    writer.writeU16(packet.reliableId());
    writer.writeU16(0);
    writeBody(writer, packet);
    writer.patchU16(PAYLOAD_SIZE_OFFSET, static_cast<std::uint16_t>(writer.size() - Protocol::HEADER_SIZE));
    return writer.release();
}

std::optional<Packet> Codec::decode(std::span<const std::uint8_t> bytes, Receiver receiver) const
{
    if (bytes.size() < Protocol::HEADER_SIZE || bytes.size() > Protocol::MAX_PACKET_BYTES)
        return std::nullopt;
    try {
        net::ByteReader reader(bytes);
        if (reader.readU16() != Protocol::MAGIC)
            return std::nullopt;
        const std::uint8_t version = reader.readU8();
        const std::uint8_t rawType = reader.readU8();
        const bool toClient = (rawType & TO_CLIENT_BIT) != 0;
        if (toClient != (receiver == Receiver::Client))
            return std::nullopt;
        const auto type = static_cast<MessageType>(rawType);
        const std::uint32_t sequence = reader.readU32();
        const std::uint32_t ack = reader.readU32();
        const std::uint32_t ackBits = reader.readU32();
        const std::uint16_t reliableId = reader.readU16();
        if (reader.readU16() != bytes.size() - Protocol::HEADER_SIZE)
            return std::nullopt;
        if (version != Protocol::VERSION && type != MessageType::Connect && type != MessageType::Reject)
            return std::nullopt;
        Packet packet = readBody(reader, type, sequence, version);
        packet.setAck(ack);
        packet.setAckBits(ackBits);
        packet.setReliableId(reliableId);
        if (reader.remaining() != 0 || !isValid(packet))
            return std::nullopt;
        return packet;
    } catch (const net::NetError &) {
        return std::nullopt;
    }
}

bool Codec::isValid(const Packet &packet) const
{
    return packet.visit(MessageValidator(m_settings));
}

void Codec::writeBody(net::ByteWriter &writer, const Packet &packet)
{
    packet.visit(MessageWriter(writer));
}

Packet Codec::readBody(net::ByteReader &reader, MessageType type, std::uint32_t sequence, std::uint8_t version)
{
    switch (type) {
    case MessageType::Connect:
        return readAs<ConnectMessage>(reader, sequence, version);
    case MessageType::Input:
        return readAs<InputMessage>(reader, sequence, version);
    case MessageType::Heartbeat:
        return readAs<HeartbeatMessage>(reader, sequence, version);
    case MessageType::Disconnect:
        return readAs<DisconnectMessage>(reader, sequence, version);
    case MessageType::Accept:
        return readAs<AcceptMessage>(reader, sequence, version);
    case MessageType::Reject:
        return readAs<RejectMessage>(reader, sequence, version);
    case MessageType::Snapshot:
        return readAs<SnapshotMessage>(reader, sequence, version);
    case MessageType::HeartbeatAck:
        return readAs<HeartbeatAckMessage>(reader, sequence, version);
    case MessageType::EntityDestroyed:
        return readAs<EntityDestroyedMessage>(reader, sequence, version);
    case MessageType::PlayerJoined:
        return readAs<PlayerJoinedMessage>(reader, sequence, version);
    case MessageType::PlayerKilled:
        return readAs<PlayerKilledMessage>(reader, sequence, version);
    case MessageType::PlayerLeft:
        return readAs<PlayerLeftMessage>(reader, sequence, version);
    case MessageType::ServerShutdown:
        return readAs<ServerShutdownMessage>(reader, sequence, version);
    case MessageType::ListRooms:
        return readAs<ListRoomsMessage>(reader, sequence, version);
    case MessageType::CreateRoom:
        return readAs<CreateRoomMessage>(reader, sequence, version);
    case MessageType::JoinRoom:
        return readAs<JoinRoomMessage>(reader, sequence, version);
    case MessageType::LeaveRoom:
        return readAs<LeaveRoomMessage>(reader, sequence, version);
    case MessageType::StartGame:
        return readAs<StartGameMessage>(reader, sequence, version);
    case MessageType::KickPlayer:
        return readAs<KickPlayerMessage>(reader, sequence, version);
    case MessageType::DeleteRoom:
        return readAs<DeleteRoomMessage>(reader, sequence, version);
    case MessageType::RoomList:
        return readAs<RoomListMessage>(reader, sequence, version);
    case MessageType::RoomJoined:
        return readAs<RoomJoinedMessage>(reader, sequence, version);
    case MessageType::RoomRejected:
        return readAs<RoomRejectedMessage>(reader, sequence, version);
    case MessageType::RoomUpdate:
        return readAs<RoomUpdateMessage>(reader, sequence, version);
    case MessageType::GameStarted:
        return readAs<GameStartedMessage>(reader, sequence, version);
    case MessageType::Kicked:
        return readAs<KickedMessage>(reader, sequence, version);
    case MessageType::RoomClosed:
        return readAs<RoomClosedMessage>(reader, sequence, version);
    case MessageType::PlaySound:
        return readAs<PlaySoundMessage>(reader, sequence, version);
    case MessageType::ConsoleCommand:
        return readAs<ConsoleCommandMessage>(reader, sequence, version);
    case MessageType::ConsoleText:
        return readAs<ConsoleTextMessage>(reader, sequence, version);
    case MessageType::SnapshotDelta:
        return readAs<SnapshotDeltaMessage>(reader, sequence, version);
    case MessageType::Chat:
        return readAs<ChatMessage>(reader, sequence, version);
    case MessageType::ChatBroadcast:
        return readAs<ChatBroadcastMessage>(reader, sequence, version);
    }
    throw net::NetError("Codec: unknown message type");
}

} // namespace engine::network
