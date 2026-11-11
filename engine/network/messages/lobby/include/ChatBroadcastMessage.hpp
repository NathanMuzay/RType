#pragma once

#include <cstdint>
#include <string>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class ChatBroadcastMessage
{
public:
    static constexpr MessageType TYPE = MessageType::ChatBroadcast;

    ChatBroadcastMessage(std::uint8_t slot, std::string text);

    std::uint8_t slot() const;
    const std::string &text() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static ChatBroadcastMessage read(net::ByteReader &reader);

private:
    std::uint8_t m_slot;
    std::string m_text;
};

} // namespace engine::network
