#pragma once

#include <cstdint>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class RejectMessage
{
public:
    static constexpr MessageType TYPE = MessageType::Reject;

    explicit RejectMessage(std::uint8_t reason);

    std::uint8_t reason() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(engine::net::ByteWriter &writer) const;
    static RejectMessage read(engine::net::ByteReader &reader);

private:
    std::uint8_t m_reason;
};

} // namespace engine::network
