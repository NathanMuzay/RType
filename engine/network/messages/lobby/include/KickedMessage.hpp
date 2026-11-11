#pragma once

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class KickedMessage
{
public:
    static constexpr MessageType TYPE = MessageType::Kicked;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static KickedMessage read(net::ByteReader &reader);
};

} // namespace engine::network
