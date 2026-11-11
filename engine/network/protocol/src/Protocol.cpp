#include "Protocol.hpp"

namespace engine::network {

bool Protocol::isReliable(MessageType type)
{
    switch (type) {
    case MessageType::CreateRoom:
    case MessageType::JoinRoom:
    case MessageType::LeaveRoom:
    case MessageType::StartGame:
    case MessageType::KickPlayer:
    case MessageType::DeleteRoom:
    case MessageType::Chat:
    case MessageType::ChatBroadcast:
    case MessageType::EntityDestroyed:
    case MessageType::PlayerJoined:
    case MessageType::PlayerKilled:
    case MessageType::PlayerLeft:
    case MessageType::PlaySound:
    case MessageType::GameStarted:
    case MessageType::RoomJoined:
    case MessageType::RoomRejected:
    case MessageType::RoomClosed:
    case MessageType::Kicked:
        return true;
    default:
        return false;
    }
}

} // namespace engine::network
