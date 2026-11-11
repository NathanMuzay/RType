#pragma once

#include <cstdint>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class KickPlayerMessage
{
public:
    static constexpr MessageType TYPE = MessageType::KickPlayer;

    KickPlayerMessage(std::uint32_t token, std::uint8_t slot);

    std::uint32_t token() const;
    std::uint8_t slot() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static KickPlayerMessage read(net::ByteReader &reader);

private:
    std::uint32_t m_token;
    std::uint8_t m_slot;
};

} // namespace engine::network
