#pragma once

#include <cstdint>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class PlayerLeftMessage
{
public:
    static constexpr MessageType TYPE = MessageType::PlayerLeft;

    PlayerLeftMessage(std::uint8_t playerId, std::uint8_t reason);

    std::uint8_t playerId() const;
    std::uint8_t reason() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(engine::net::ByteWriter &writer) const;
    static PlayerLeftMessage read(engine::net::ByteReader &reader);

private:
    std::uint8_t m_playerId;
    std::uint8_t m_reason;
};

} // namespace engine::network
