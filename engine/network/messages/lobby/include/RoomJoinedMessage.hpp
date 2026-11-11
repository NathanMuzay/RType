#pragma once

#include <cstdint>
#include <string>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class RoomJoinedMessage
{
public:
    static constexpr MessageType TYPE = MessageType::RoomJoined;

    RoomJoinedMessage(std::uint16_t roomId, std::string name, std::uint8_t slot);

    std::uint16_t roomId() const;
    const std::string &name() const;
    std::uint8_t slot() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static RoomJoinedMessage read(net::ByteReader &reader);

private:
    std::uint16_t m_roomId;
    std::string m_name;
    std::uint8_t m_slot;
};

} // namespace engine::network
