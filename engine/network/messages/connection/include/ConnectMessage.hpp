#pragma once

#include <string>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class ConnectMessage
{
public:
    static constexpr MessageType TYPE = MessageType::Connect;

    ConnectMessage();
    explicit ConnectMessage(std::string name);

    const std::string &name() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(engine::net::ByteWriter &writer) const;
    static ConnectMessage read(engine::net::ByteReader &reader);

private:
    std::string m_name;
};

} // namespace engine::network
