#include "EntityChange.hpp"

namespace engine::network {

EntityChange::EntityChange(
    std::uint16_t id, std::uint8_t fields, std::int16_t x, std::int16_t y, std::uint8_t kind, std::uint8_t visual)
    : m_id(id), m_fields(fields), m_x(x), m_y(y), m_kind(kind), m_visual(visual)
{
}

EntityChange EntityChange::created(const EntityState &entity)
{
    return EntityChange(entity.id(), FIELD_POSITION | FIELD_KIND | FIELD_VISUAL, entity.x(), entity.y(), entity.kind(),
        entity.visual());
}

std::optional<EntityChange> EntityChange::between(const EntityState &base, const EntityState &current)
{
    std::uint8_t fields = 0;
    const std::int32_t stepX = static_cast<std::int32_t>(current.x()) - base.x();
    const std::int32_t stepY = static_cast<std::int32_t>(current.y()) - base.y();
    const bool farX = stepX < MIN_STEP || stepX > MAX_STEP;
    const bool farY = stepY < MIN_STEP || stepY > MAX_STEP;
    if (farX || farY) {
        if (stepX != 0 || stepY != 0)
            fields |= FIELD_POSITION;
    } else {
        if (stepX != 0)
            fields |= FIELD_DX;
        if (stepY != 0)
            fields |= FIELD_DY;
    }
    if (current.kind() != base.kind())
        fields |= FIELD_KIND;
    if (current.visual() != base.visual())
        fields |= FIELD_VISUAL;
    if (fields == 0)
        return std::nullopt;
    const std::int16_t x = (fields & FIELD_POSITION) != 0 ? current.x() : static_cast<std::int16_t>(stepX);
    const std::int16_t y = (fields & FIELD_POSITION) != 0 ? current.y() : static_cast<std::int16_t>(stepY);
    return EntityChange(current.id(), fields, x, y, current.kind(), current.visual());
}

std::uint16_t EntityChange::id() const
{
    return m_id;
}

std::uint8_t EntityChange::fields() const
{
    return m_fields;
}

bool EntityChange::isValid() const
{
    if (m_id == 0 || m_fields == 0 || (m_fields & ~KNOWN_FIELDS) != 0)
        return false;
    return (m_fields & FIELD_POSITION) == 0 || (m_fields & (FIELD_DX | FIELD_DY)) == 0;
}

std::optional<EntityState> EntityChange::resolve(const EntityState *base) const
{
    if ((m_fields & FIELD_POSITION) != 0)
        return EntityState(m_id, (m_fields & FIELD_KIND) != 0 ? m_kind : (base ? base->kind() : 0), m_x, m_y,
            (m_fields & FIELD_VISUAL) != 0 ? m_visual : (base ? base->visual() : 0));
    if (!base)
        return std::nullopt;
    const std::int16_t x = (m_fields & FIELD_DX) != 0 ? static_cast<std::int16_t>(base->x() + m_x) : base->x();
    const std::int16_t y = (m_fields & FIELD_DY) != 0 ? static_cast<std::int16_t>(base->y() + m_y) : base->y();
    return EntityState(m_id, (m_fields & FIELD_KIND) != 0 ? m_kind : base->kind(), x, y,
        (m_fields & FIELD_VISUAL) != 0 ? m_visual : base->visual());
}

void EntityChange::write(net::ByteWriter &writer) const
{
    writer.writeU16(m_id);
    writer.writeU8(m_fields);
    if ((m_fields & FIELD_POSITION) != 0) {
        writer.writeI16(m_x);
        writer.writeI16(m_y);
    } else {
        if ((m_fields & FIELD_DX) != 0)
            writer.writeU8(static_cast<std::uint8_t>(static_cast<std::int8_t>(m_x)));
        if ((m_fields & FIELD_DY) != 0)
            writer.writeU8(static_cast<std::uint8_t>(static_cast<std::int8_t>(m_y)));
    }
    if ((m_fields & FIELD_KIND) != 0)
        writer.writeU8(m_kind);
    if ((m_fields & FIELD_VISUAL) != 0)
        writer.writeU8(m_visual);
}

EntityChange EntityChange::read(net::ByteReader &reader)
{
    const std::uint16_t id = reader.readU16();
    const std::uint8_t fields = reader.readU8();
    std::int16_t x = 0;
    std::int16_t y = 0;
    if ((fields & FIELD_POSITION) != 0) {
        x = reader.readI16();
        y = reader.readI16();
    } else {
        if ((fields & FIELD_DX) != 0)
            x = static_cast<std::int8_t>(reader.readU8());
        if ((fields & FIELD_DY) != 0)
            y = static_cast<std::int8_t>(reader.readU8());
    }
    const std::uint8_t kind = (fields & FIELD_KIND) != 0 ? reader.readU8() : 0;
    const std::uint8_t visual = (fields & FIELD_VISUAL) != 0 ? reader.readU8() : 0;
    return EntityChange(id, fields, x, y, kind, visual);
}

} // namespace engine::network
