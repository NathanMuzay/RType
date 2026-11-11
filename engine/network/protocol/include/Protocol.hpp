#pragma once

#include <cstddef>
#include <cstdint>

namespace engine::network {

enum class MessageType : std::uint8_t
{
    Connect = 0x01,
    Input = 0x02,
    Heartbeat = 0x03,
    Disconnect = 0x04,
    ListRooms = 0x05,
    CreateRoom = 0x06,
    JoinRoom = 0x07,
    LeaveRoom = 0x08,
    StartGame = 0x09,
    KickPlayer = 0x0A,
    DeleteRoom = 0x0B,
    ConsoleCommand = 0x0C,
    Chat = 0x0D,
    Accept = 0x81,
    Reject = 0x82,
    Snapshot = 0x83,
    HeartbeatAck = 0x84,
    EntityDestroyed = 0x85,
    PlayerJoined = 0x86,
    PlayerKilled = 0x87,
    PlayerLeft = 0x88,
    ServerShutdown = 0x89,
    RoomList = 0x8A,
    RoomJoined = 0x8B,
    RoomRejected = 0x8C,
    RoomUpdate = 0x8D,
    GameStarted = 0x8E,
    Kicked = 0x8F,
    RoomClosed = 0x90,
    PlaySound = 0x91,
    ConsoleText = 0x92,
    SnapshotDelta = 0x93,
    ChatBroadcast = 0x94
};

enum class Receiver
{
    Server,
    Client
};

class Protocol
{
public:
    static bool isReliable(MessageType type);

    static constexpr std::uint16_t MAGIC = 0x454E;
    static constexpr std::uint8_t VERSION = 9;
    static constexpr std::size_t HEADER_SIZE = 20;
    static constexpr std::size_t MAX_PACKET_BYTES = 1200;

    static constexpr std::uint8_t BUTTON_UP = 0x01;
    static constexpr std::uint8_t BUTTON_DOWN = 0x02;
    static constexpr std::uint8_t BUTTON_LEFT = 0x04;
    static constexpr std::uint8_t BUTTON_RIGHT = 0x08;
    static constexpr std::uint8_t BUTTON_FIRE = 0x10;
    static constexpr std::uint8_t BUTTON_ACTION = 0x20;
    static constexpr std::uint8_t BUTTON_BOMB = 0x40;
    static constexpr std::uint8_t RESERVED_BUTTONS = 0x80;

    static constexpr std::uint8_t REJECT_SERVER_FULL = 1;
    static constexpr std::uint8_t REJECT_VERSION_MISMATCH = 2;
    static constexpr std::uint8_t LEFT_QUIT = 1;
    static constexpr std::uint8_t LEFT_TIMEOUT = 2;

    static constexpr std::uint8_t ROOM_NOT_FOUND = 1;
    static constexpr std::uint8_t ROOM_FULL = 2;
    static constexpr std::uint8_t ROOM_WRONG_CODE = 3;
    static constexpr std::uint8_t ROOM_LIMIT_REACHED = 4;
    static constexpr std::uint8_t ROOM_INVALID_NAME = 5;
    static constexpr std::uint8_t ROOM_TOO_MANY_ATTEMPTS = 6;
    static constexpr std::uint8_t ROOM_NOT_ALLOWED = 7;

    static constexpr std::uint8_t SLOT_EMPTY = 0;
    static constexpr std::uint8_t SLOT_ALIVE = 1;
    static constexpr std::uint8_t SLOT_DEAD = 2;

    static constexpr std::uint8_t GAME_WAITING = 0;
    static constexpr std::uint8_t GAME_PLAYING = 1;
    static constexpr std::uint8_t GAME_OVER = 2;

    static constexpr std::uint8_t MAX_PERCENT = 100;
    static constexpr std::size_t MAX_WORLD_VALUES = 32;
    static constexpr std::uint8_t VISUAL_FLASH = 0x80;
    static constexpr std::uint8_t VISUAL_ANGLE_MASK = 0x7F;
    static constexpr float VISUAL_ANGLE_STEPS = 128.f;
};

} // namespace engine::network
