#include "ChatMessage.hpp"

#include <utility>

#include "TextRules.hpp"

namespace engine::network {

ChatMessage::ChatMessage(std::uint32_t token, std::string text) : m_token(token), m_text(std::move(text))
{
}

std::uint32_t ChatMessage::token() const
{
    return m_token;
}

const std::string &ChatMessage::text() const
{
    return m_text;
}

bool ChatMessage::isValid(const NetworkSettings &settings) const
{
    return m_token != 0 && TextRules::isValid(m_text, 1, settings.chatMaxLength());
}

void ChatMessage::write(net::ByteWriter &writer) const
{
    writer.writeU32(m_token);
    writer.writeString(m_text);
}

ChatMessage ChatMessage::read(net::ByteReader &reader)
{
    const std::uint32_t token = reader.readU32();
    return ChatMessage(token, reader.readString());
}

} // namespace engine::network
