#include "ConsoleTextMessage.hpp"

#include <utility>

#include "TextRules.hpp"

namespace engine::network {

ConsoleTextMessage::ConsoleTextMessage(std::string text) : m_text(std::move(text))
{
}

const std::string &ConsoleTextMessage::text() const
{
    return m_text;
}

bool ConsoleTextMessage::isValid(const NetworkSettings &) const
{
    return TextRules::isValid(m_text, 0, MAX_LENGTH);
}

void ConsoleTextMessage::write(net::ByteWriter &writer) const
{
    writer.writeString(m_text);
}

ConsoleTextMessage ConsoleTextMessage::read(net::ByteReader &reader)
{
    return ConsoleTextMessage(reader.readString());
}

} // namespace engine::network
