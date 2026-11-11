#include "JoinRoomMessage.hpp"

#include <utility>

#include "TextRules.hpp"

namespace engine::network {

JoinRoomMessage::JoinRoomMessage(std::uint32_t token, std::uint16_t roomId, std::string code)
    : m_token(token), m_roomId(roomId), m_code(std::move(code))
{
}

std::uint32_t JoinRoomMessage::token() const
{
    return m_token;
}

std::uint16_t JoinRoomMessage::roomId() const
{
    return m_roomId;
}

const std::string &JoinRoomMessage::code() const
{
    return m_code;
}

bool JoinRoomMessage::isValid(const NetworkSettings &settings) const
{
    return m_token != 0 && TextRules::isValid(m_code, 0, settings.roomCodeMaxLength());
}

void JoinRoomMessage::write(net::ByteWriter &writer) const
{
    writer.writeU32(m_token);
    writer.writeU16(m_roomId);
    writer.writeString(m_code);
}

JoinRoomMessage JoinRoomMessage::read(net::ByteReader &reader)
{
    const std::uint32_t token = reader.readU32();
    const std::uint16_t roomId = reader.readU16();
    std::string code = reader.readString();
    return JoinRoomMessage(token, roomId, std::move(code));
}

} // namespace engine::network
