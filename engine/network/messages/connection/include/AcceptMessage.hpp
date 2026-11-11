#pragma once

#include <cstdint>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class AcceptMessage
{
public:
    static constexpr MessageType TYPE = MessageType::Accept;

    AcceptMessage(std::uint32_t token, std::uint8_t playerId);

    std::uint32_t token() const;
    std::uint8_t playerId() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(engine::net::ByteWriter &writer) const;
    static AcceptMessage read(engine::net::ByteReader &reader);

private:
    std::uint32_t m_token;
    std::uint8_t m_playerId;
};

} // namespace engine::network
