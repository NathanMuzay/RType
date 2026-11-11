#pragma once

namespace engine::network {

template <class Message>
bool MessageValidator::operator()(const Message &message) const
{
    return message.isValid(m_settings);
}

template <class Message>
void MessageWriter::operator()(const Message &message) const
{
    message.write(m_writer);
}

template <class Message>
std::optional<std::uint32_t> TokenReader::operator()(const Message &message) const
{
    if constexpr (requires { message.token(); })
        return message.token();
    else
        return std::nullopt;
}

} // namespace engine::network
