#include "ConsoleCommandMessage.hpp"

#include <utility>

#include "TextRules.hpp"

namespace engine::network {

ConsoleCommandMessage::ConsoleCommandMessage(std::uint32_t token, std::string text)
    : m_token(token), m_text(std::move(text))
{
}

std::uint32_t ConsoleCommandMessage::token() const
{
    return m_token;
}

const std::string &ConsoleCommandMessage::text() const
{
    return m_text;
}

bool ConsoleCommandMessage::isValid(const NetworkSettings &) const
{
    return m_token != 0 && TextRules::isValid(m_text, 1, MAX_LENGTH);
}

void ConsoleCommandMessage::write(net::ByteWriter &writer) const
{
    writer.writeU32(m_token);
    writer.writeString(m_text);
}

ConsoleCommandMessage ConsoleCommandMessage::read(net::ByteReader &reader)
{
    const std::uint32_t token = reader.readU32();
    std::string text = reader.readString();
    return ConsoleCommandMessage(token, std::move(text));
}

} // namespace engine::network
