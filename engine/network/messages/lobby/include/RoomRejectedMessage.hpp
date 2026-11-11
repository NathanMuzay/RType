#pragma once

#include <cstdint>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class RoomRejectedMessage
{
public:
    static constexpr MessageType TYPE = MessageType::RoomRejected;

    explicit RoomRejectedMessage(std::uint8_t reason);

    std::uint8_t reason() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static RoomRejectedMessage read(net::ByteReader &reader);

private:
    std::uint8_t m_reason;
};

} // namespace engine::network
