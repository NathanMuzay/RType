#include "GameServer.hpp"

#include "PacketVisitors.hpp"

#include <iostream>
#include <utility>

namespace engine::network {

void GameServer::handleCreateRoom(std::size_t client, const CreateRoomMessage &request)
{
    if (GameRoom *current = roomOf(client)) {
        sendRoomJoined(client, *current);
        return;
    }
    if (m_rooms.size() >= m_settings.maxRooms()) {
        sendTo(client, Packet(RoomRejectedMessage(Protocol::ROOM_LIMIT_REACHED)));
        return;
    }
    const std::uint16_t id = newRoomId();
    GameRoom &room = m_rooms
                         .try_emplace(id, id, request.name(), request.code(), m_settings.roomCapacity(request.rules()),
                             m_factory.createSession(request.rules()), request.rules())
                         .first->second;
    seatClient(client, room, GameRoom::OWNER_SLOT);
    std::cout << "Client " << client << " created room " << id << " (" << request.name() << ")" << std::endl;
}

void GameServer::handleJoinRoom(std::size_t client, const JoinRoomMessage &request)
{
    if (GameRoom *current = roomOf(client)) {
        sendRoomJoined(client, *current);
        return;
    }
    ClientSession &session = *m_clients[client];
    const auto found = m_rooms.find(request.roomId());
    std::uint8_t reason = 0;
    if (session.failedJoins() >= m_settings.maxJoinAttempts())
        reason = Protocol::ROOM_TOO_MANY_ATTEMPTS;
    else if (found == m_rooms.end())
        reason = Protocol::ROOM_NOT_FOUND;
    else if (!found->second.freeSlot())
        reason = Protocol::ROOM_FULL;
    else if (!found->second.acceptsCode(request.code()))
        reason = Protocol::ROOM_WRONG_CODE;
    if (reason == Protocol::ROOM_WRONG_CODE)
        session.addFailedJoin();
    if (reason != 0) {
        sendTo(client, Packet(RoomRejectedMessage(reason)));
        return;
    }
    GameRoom &room = found->second;
    seatClient(client, room, *room.freeSlot());
}

void GameServer::handleStartGame(std::size_t client)
{
    GameRoom *room = roomOf(client);
    if (!ownedRoomSlot(client, room) || room->playing())
        return;
    room->setPlaying(true);
    for (std::size_t slot = 0; slot < room->capacity(); ++slot) {
        if (room->clientAt(slot))
            room->session().addPlayer(slot);
    }
    for (const auto &[slot, text] : room->takeCommands())
        room->session().command(slot, text);
    broadcastRoom(*room, Packet(GameStartedMessage()));
    std::cout << "Room " << room->id() << " started" << std::endl;
}

void GameServer::handleKick(std::size_t client, const KickPlayerMessage &request)
{
    GameRoom *room = roomOf(client);
    const auto ownerSlot = ownedRoomSlot(client, room);
    if (!ownerSlot || request.slot() == *ownerSlot)
        return;
    const auto target = room->clientAt(request.slot());
    if (!target)
        return;
    sendTo(*target, Packet(KickedMessage()));
    leaveRoom(*target);
}

void GameServer::handleConsole(std::size_t client, const ConsoleCommandMessage &request)
{
    GameRoom *room = roomOf(client);
    const auto slot = room ? room->slotOf(client) : std::nullopt;
    if (!slot || *slot != room->owner()) {
        sendTo(client, Packet(ConsoleTextMessage("server commands are reserved to the room owner")));
        return;
    }
    const auto rule = m_settings.consoleRule();
    if (rule && room->rule(*rule) == 0 && !m_settings.isFreeCommand(request.text())) {
        sendTo(client, Packet(ConsoleTextMessage("server commands are disabled in this room (cheats OFF)")));
        return;
    }
    if (!room->playing()) {
        room->queueCommand(*slot, request.text());
        sendTo(client, Packet(ConsoleTextMessage("queued: runs when the game starts")));
        return;
    }
    room->session().command(*slot, request.text());
}

void GameServer::handleDeleteRoom(std::size_t client)
{
    GameRoom *room = roomOf(client);
    if (!ownedRoomSlot(client, room))
        return;
    broadcastRoom(*room, Packet(RoomClosedMessage()));
    closeRoom(room->id());
}

void GameServer::handleChat(std::size_t client, const ChatMessage &request, Time now)
{
    GameRoom *room = roomOf(client);
    if (!room)
        return;
    ClientSession &session = *m_clients[client];
    if (now - session.lastChat() < m_settings.chatInterval())
        return;
    const auto slot = room->slotOf(client);
    if (!slot)
        return;
    session.markChat(now);
    broadcastRoom(*room, Packet(ChatBroadcastMessage(static_cast<std::uint8_t>(*slot), request.text())));
}

void GameServer::seatClient(std::size_t client, GameRoom &room, std::size_t slot)
{
    room.seat(slot, client);
    m_clients[client]->enterRoom(room.id());
    m_clients[client]->snapshots().forget();
    if (room.playing()) {
        room.session().addPlayer(slot);
        sendRoomJoined(client, room);
        sendTo(client, Packet(GameStartedMessage()));
    } else {
        sendRoomJoined(client, room);
    }
    sendRoomUpdate(room);
}

void GameServer::leaveRoom(std::size_t client, std::uint8_t reason)
{
    GameRoom *room = roomOf(client);
    m_clients[client]->leaveRoom();
    m_clients[client]->snapshots().forget();
    if (!room)
        return;
    const auto slot = room->slotOf(client);
    if (!slot)
        return;
    if (*slot == room->owner()) {
        room->unseat(*slot);
        broadcastRoom(*room, Packet(RoomClosedMessage()));
        closeRoom(room->id());
        return;
    }
    const bool playing = room->playing();
    if (playing)
        room->session().removePlayer(*slot);
    room->unseat(*slot);
    if (playing)
        broadcastRoom(*room, Packet(PlayerLeftMessage(static_cast<std::uint8_t>(*slot), reason)));
    sendRoomUpdate(*room);
}

void GameServer::closeRoom(std::uint16_t roomId)
{
    const auto found = m_rooms.find(roomId);
    if (found == m_rooms.end())
        return;
    for (std::size_t member : found->second.clients()) {
        if (m_clients[member])
            m_clients[member]->leaveRoom();
    }
    m_rooms.erase(found);
    std::cout << "Room " << roomId << " closed" << std::endl;
}

GameRoom *GameServer::roomOf(std::size_t client)
{
    if (!m_clients[client])
        return nullptr;
    const auto roomId = m_clients[client]->roomId();
    if (!roomId)
        return nullptr;
    const auto found = m_rooms.find(*roomId);
    return found == m_rooms.end() ? nullptr : &found->second;
}

std::optional<std::size_t> GameServer::ownedRoomSlot(std::size_t client, GameRoom *room)
{
    if (!room)
        return std::nullopt;
    const auto slot = room->slotOf(client);
    if (!slot || *slot != room->owner()) {
        sendTo(client, Packet(RoomRejectedMessage(Protocol::ROOM_NOT_ALLOWED)));
        return std::nullopt;
    }
    return slot;
}

std::uint16_t GameServer::newRoomId()
{
    while (m_nextRoomId == 0 || m_rooms.contains(m_nextRoomId))
        ++m_nextRoomId;
    return m_nextRoomId++;
}

void GameServer::sendRoomList(std::size_t client)
{
    std::vector<RoomInfo> rooms;
    for (const auto &entry : m_rooms) {
        const GameRoom &room = entry.second;
        rooms.emplace_back(room.id(), room.name(), static_cast<std::uint8_t>(room.playerCount()),
            static_cast<std::uint8_t>(room.capacity()), room.locked(), room.playing());
    }
    sendTo(client, Packet(RoomListMessage(std::move(rooms))));
}

void GameServer::sendRoomJoined(std::size_t client, GameRoom &room)
{
    const auto slot = room.slotOf(client);
    if (slot)
        sendTo(client, Packet(RoomJoinedMessage(room.id(), room.name(), static_cast<std::uint8_t>(*slot))));
}

void GameServer::sendRoomUpdate(GameRoom &room)
{
    std::vector<std::string> names;
    for (std::size_t slot = 0; slot < room.capacity(); ++slot) {
        const auto client = room.clientAt(slot);
        names.push_back(client && m_clients[*client] ? m_clients[*client]->name() : std::string());
    }
    broadcastRoom(room,
        Packet(RoomUpdateMessage(static_cast<std::uint8_t>(room.owner()), room.occupiedSlots(), std::move(names))));
}

void GameServer::broadcastRoom(GameRoom &room, const Packet &packet)
{
    for (std::size_t client : room.clients())
        sendTo(client, packet);
}

} // namespace engine::network
