#pragma once

#include <cstdint>
#include <string>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class JoinRoomMessage
{
public:
    static constexpr MessageType TYPE = MessageType::JoinRoom;

    JoinRoomMessage(std::uint32_t token, std::uint16_t roomId, std::string code);

    std::uint32_t token() const;
    std::uint16_t roomId() const;
    const std::string &code() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static JoinRoomMessage read(net::ByteReader &reader);

private:
    std::uint32_t m_token;
    std::uint16_t m_roomId;
    std::string m_code;
};

} // namespace engine::network
