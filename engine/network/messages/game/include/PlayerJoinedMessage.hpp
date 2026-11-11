#pragma once

#include <cstdint>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class PlayerJoinedMessage
{
public:
    static constexpr MessageType TYPE = MessageType::PlayerJoined;

    explicit PlayerJoinedMessage(std::uint8_t playerId);

    std::uint8_t playerId() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(engine::net::ByteWriter &writer) const;
    static PlayerJoinedMessage read(engine::net::ByteReader &reader);

private:
    std::uint8_t m_playerId;
};

} // namespace engine::network
