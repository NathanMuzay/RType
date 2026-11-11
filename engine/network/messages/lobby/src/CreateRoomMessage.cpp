#include "CreateRoomMessage.hpp"

#include <utility>

#include "TextRules.hpp"

namespace engine::network {

CreateRoomMessage::CreateRoomMessage(
    std::uint32_t token, std::string name, std::string code, std::vector<std::uint8_t> rules)
    : m_token(token), m_name(std::move(name)), m_code(std::move(code)), m_rules(std::move(rules))
{
}

std::uint32_t CreateRoomMessage::token() const
{
    return m_token;
}

const std::string &CreateRoomMessage::name() const
{
    return m_name;
}

const std::string &CreateRoomMessage::code() const
{
    return m_code;
}

const std::vector<std::uint8_t> &CreateRoomMessage::rules() const
{
    return m_rules;
}

bool CreateRoomMessage::isValid(const NetworkSettings &settings) const
{
    return m_token != 0 && TextRules::isValid(m_name, 1, settings.roomNameMaxLength())
        && TextRules::isValid(m_code, 0, settings.roomCodeMaxLength()) && m_rules.size() == settings.ruleCount();
}

void CreateRoomMessage::write(net::ByteWriter &writer) const
{
    writer.writeU32(m_token);
    writer.writeString(m_name);
    writer.writeString(m_code);
    writer.writeU8(static_cast<std::uint8_t>(m_rules.size()));
    for (std::uint8_t rule : m_rules)
        writer.writeU8(rule);
}

CreateRoomMessage CreateRoomMessage::read(net::ByteReader &reader)
{
    const std::uint32_t token = reader.readU32();
    std::string name = reader.readString();
    std::string code = reader.readString();
    const std::size_t count = reader.readU8();
    std::vector<std::uint8_t> rules;
    rules.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
        rules.push_back(reader.readU8());
    return CreateRoomMessage(token, std::move(name), std::move(code), std::move(rules));
}

} // namespace engine::network
