#pragma once

#include <cstdint>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class HeartbeatAckMessage
{
public:
    static constexpr MessageType TYPE = MessageType::HeartbeatAck;

    explicit HeartbeatAckMessage(std::uint32_t timestamp);

    std::uint32_t timestamp() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(engine::net::ByteWriter &writer) const;
    static HeartbeatAckMessage read(engine::net::ByteReader &reader);

private:
    std::uint32_t m_timestamp;
};

} // namespace engine::network
