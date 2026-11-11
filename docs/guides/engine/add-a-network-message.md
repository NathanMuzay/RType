# Add a network message

Most features need no new message: world values, player values, sounds and console text already travel in
existing messages. Add a message only for a new kind of exchange. This guide uses the real `PLAY_SOUND`.

## 1. Choose a code

In `engine/network/protocol/include/Protocol.hpp`, add a value to `MessageType`. Client messages use
`0x01`-`0x7F`, server messages `0x81`-`0xFF` (the `0x80` bit is the direction):

```cpp
PlaySound = 0x91,
```

## 2. Write the message class

In `engine/network/messages/<category>/include/` and `src/` (`connection`, `lobby`, `game` or `console`):

```cpp
class PlaySoundMessage
{
public:
    static constexpr MessageType TYPE = MessageType::PlaySound;

    explicit PlaySoundMessage(std::uint8_t sound);

    std::uint8_t sound() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static PlaySoundMessage read(net::ByteReader &reader);

private:
    std::uint8_t m_sound;
};
```

```cpp
bool PlaySoundMessage::isValid(const NetworkSettings &settings) const
{
    return m_sound < settings.eventSoundCount();
}

void PlaySoundMessage::write(engine::net::ByteWriter &writer) const
{
    writer.writeU8(m_sound);
}

PlaySoundMessage PlaySoundMessage::read(engine::net::ByteReader &reader)
{
    return PlaySoundMessage(reader.readU8());
}
```

`isValid` must reject every value the receiver cannot use: it runs on both sending and receiving.
Client messages start with the session token (`writeU32(m_token)`), and `isValid` checks `m_token != 0`.

## 3. Register it

| File | Change |
|---|---|
| `network/protocol/include/Packet.hpp` | Add the class to the `Body` variant |
| `network/protocol/src/Codec.cpp` | Add a `case` to `readBody` |
| `network/protocol/include/Packet.hpp` | Include the new header with the other messages |
| `engine/CMakeLists.txt` | Add the `.cpp` to `engine_network`, and its `include` folder if it is a new category |

`MessageValidator`, `MessageWriter` and `TokenReader` work with any message automatically.

## 4. Send and handle it

- Server side: `GameServer` sends with `sendTo(client, Packet(PlaySoundMessage(index)))` or `broadcastRoom`.
  Messages produced by the simulation go through `Simulation::takeEvents()`.
- Client side: `OnlineClient::handleEvent` tests `event.holds<PlaySoundMessage>()`, then reads
  `event.as<PlaySoundMessage>()`.
- If the client must send it, add a function to `ClientConnection` (use `sendIfConnected`).

## 5. Version and document

1. Increase `Protocol::VERSION`: old clients are then refused cleanly.
2. Add the message to the [RFC](../../reference/protocol.md): code table, payload table, validation rules.
3. Test with a client and a server built from the same commit, and with an old client (it must get `REJECT(2)`).
