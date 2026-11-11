#include "InputMessage.hpp"

namespace engine::network {

InputMessage::InputMessage(std::uint32_t token, std::uint8_t buttons) : m_token(token), m_buttons(buttons)
{
}

std::uint32_t InputMessage::token() const
{
    return m_token;
}

std::uint8_t InputMessage::buttons() const
{
    return m_buttons;
}

bool InputMessage::isValid(const NetworkSettings &) const
{
    return m_token != 0 && (m_buttons & Protocol::RESERVED_BUTTONS) == 0;
}

void InputMessage::write(engine::net::ByteWriter &writer) const
{
    writer.writeU32(m_token);
    writer.writeU8(m_buttons);
}

InputMessage InputMessage::read(engine::net::ByteReader &reader)
{
    const std::uint32_t token = reader.readU32();
    const std::uint8_t buttons = reader.readU8();
    return InputMessage(token, buttons);
}

} // namespace engine::network
