#include "ConnectMessage.hpp"

#include <utility>

#include "TextRules.hpp"

namespace engine::network {

ConnectMessage::ConnectMessage() : m_name()
{
}

ConnectMessage::ConnectMessage(std::string name) : m_name(std::move(name))
{
}

const std::string &ConnectMessage::name() const
{
    return m_name;
}

bool ConnectMessage::isValid(const NetworkSettings &settings) const
{
    return TextRules::isValid(m_name, 0, settings.playerNameMaxLength());
}

void ConnectMessage::write(engine::net::ByteWriter &writer) const
{
    writer.writeString(m_name);
}

ConnectMessage ConnectMessage::read(engine::net::ByteReader &reader)
{
    return ConnectMessage(reader.readString());
}

} // namespace engine::network
