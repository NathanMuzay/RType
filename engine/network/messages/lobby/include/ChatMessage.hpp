#pragma once

#include <cstdint>
#include <string>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class ChatMessage
{
public:
    static constexpr MessageType TYPE = MessageType::Chat;

    ChatMessage(std::uint32_t token, std::string text);

    std::uint32_t token() const;
    const std::string &text() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static ChatMessage read(net::ByteReader &reader);

private:
    std::uint32_t m_token;
    std::string m_text;
};

} // namespace engine::network
