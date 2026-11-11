#pragma once

#include <string>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class ConsoleTextMessage
{
public:
    static constexpr MessageType TYPE = MessageType::ConsoleText;
    static constexpr std::size_t MAX_LENGTH = 200;

    explicit ConsoleTextMessage(std::string text);

    const std::string &text() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static ConsoleTextMessage read(net::ByteReader &reader);

private:
    std::string m_text;
};

} // namespace engine::network
