#include "EntityState.hpp"

namespace engine::network {

EntityState::EntityState(std::uint16_t id, std::uint8_t kind, std::int16_t x, std::int16_t y, std::uint8_t visual)
    : m_id(id), m_kind(kind), m_x(x), m_y(y), m_visual(visual)
{
}

std::uint16_t EntityState::id() const
{
    return m_id;
}

std::uint8_t EntityState::kind() const
{
    return m_kind;
}

std::int16_t EntityState::x() const
{
    return m_x;
}

std::int16_t EntityState::y() const
{
    return m_y;
}

std::uint8_t EntityState::visual() const
{
    return m_visual;
}

bool EntityState::operator==(const EntityState &other) const
{
    return m_id == other.m_id && m_kind == other.m_kind && m_x == other.m_x && m_y == other.m_y
        && m_visual == other.m_visual;
}

void EntityState::write(engine::net::ByteWriter &writer) const
{
    writer.writeU16(m_id);
    writer.writeU8(m_kind);
    writer.writeI16(m_x);
    writer.writeI16(m_y);
    writer.writeU8(m_visual);
}

EntityState EntityState::read(engine::net::ByteReader &reader)
{
    const std::uint16_t id = reader.readU16();
    const std::uint8_t kind = reader.readU8();
    const std::int16_t x = reader.readI16();
    const std::int16_t y = reader.readI16();
    const std::uint8_t visual = reader.readU8();
    return EntityState(id, kind, x, y, visual);
}

} // namespace engine::network
