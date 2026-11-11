#pragma once

#include <cstdint>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class DisconnectMessage
{
public:
    static constexpr MessageType TYPE = MessageType::Disconnect;

    explicit DisconnectMessage(std::uint32_t token);

    std::uint32_t token() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(engine::net::ByteWriter &writer) const;
    static DisconnectMessage read(engine::net::ByteReader &reader);

private:
    std::uint32_t m_token;
};

} // namespace engine::network
