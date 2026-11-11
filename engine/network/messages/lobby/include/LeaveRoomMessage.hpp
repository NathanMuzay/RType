#pragma once

#include <cstdint>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class LeaveRoomMessage
{
public:
    static constexpr MessageType TYPE = MessageType::LeaveRoom;

    explicit LeaveRoomMessage(std::uint32_t token);

    std::uint32_t token() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static LeaveRoomMessage read(net::ByteReader &reader);

private:
    std::uint32_t m_token;
};

} // namespace engine::network
