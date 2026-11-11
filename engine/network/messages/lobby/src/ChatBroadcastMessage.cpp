#include "ChatBroadcastMessage.hpp"

#include <utility>

#include "TextRules.hpp"

namespace engine::network {

ChatBroadcastMessage::ChatBroadcastMessage(std::uint8_t slot, std::string text) : m_slot(slot), m_text(std::move(text))
{
}

std::uint8_t ChatBroadcastMessage::slot() const
{
    return m_slot;
}

const std::string &ChatBroadcastMessage::text() const
{
    return m_text;
}

bool ChatBroadcastMessage::isValid(const NetworkSettings &settings) const
{
    return m_slot < settings.maxPlayers() && TextRules::isValid(m_text, 1, settings.chatMaxLength());
}

void ChatBroadcastMessage::write(net::ByteWriter &writer) const
{
    writer.writeU8(m_slot);
    writer.writeString(m_text);
}

ChatBroadcastMessage ChatBroadcastMessage::read(net::ByteReader &reader)
{
    const std::uint8_t slot = reader.readU8();
    return ChatBroadcastMessage(slot, reader.readString());
}

} // namespace engine::network
