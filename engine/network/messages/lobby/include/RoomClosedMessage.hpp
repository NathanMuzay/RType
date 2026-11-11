#pragma once

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class RoomClosedMessage
{
public:
    static constexpr MessageType TYPE = MessageType::RoomClosed;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static RoomClosedMessage read(net::ByteReader &reader);
};

} // namespace engine::network
