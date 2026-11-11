#pragma once

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class ServerShutdownMessage
{
public:
    static constexpr MessageType TYPE = MessageType::ServerShutdown;

    bool isValid(const NetworkSettings &settings) const;
    void write(engine::net::ByteWriter &writer) const;
    static ServerShutdownMessage read(engine::net::ByteReader &reader);
};

} // namespace engine::network
