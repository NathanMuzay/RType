#pragma once

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class GameStartedMessage
{
public:
    static constexpr MessageType TYPE = MessageType::GameStarted;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static GameStartedMessage read(net::ByteReader &reader);
};

} // namespace engine::network
