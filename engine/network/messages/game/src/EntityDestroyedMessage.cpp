#include "EntityDestroyedMessage.hpp"

namespace engine::network {

EntityDestroyedMessage::EntityDestroyedMessage(std::uint16_t entityId) : m_entityId(entityId)
{
}

std::uint16_t EntityDestroyedMessage::entityId() const
{
    return m_entityId;
}

bool EntityDestroyedMessage::isValid(const NetworkSettings &) const
{
    return m_entityId != 0;
}

void EntityDestroyedMessage::write(engine::net::ByteWriter &writer) const
{
    writer.writeU16(m_entityId);
}

EntityDestroyedMessage EntityDestroyedMessage::read(engine::net::ByteReader &reader)
{
    return EntityDestroyedMessage(reader.readU16());
}

} // namespace engine::network
