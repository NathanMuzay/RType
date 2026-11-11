#pragma once

#include <cstdint>
#include <optional>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "EntityState.hpp"

namespace engine::network {

class EntityChange
{
public:
    static constexpr std::uint8_t FIELD_DX = 0x01;
    static constexpr std::uint8_t FIELD_DY = 0x02;
    static constexpr std::uint8_t FIELD_POSITION = 0x04;
    static constexpr std::uint8_t FIELD_KIND = 0x08;
    static constexpr std::uint8_t FIELD_VISUAL = 0x10;
    static constexpr std::uint8_t KNOWN_FIELDS = 0x1F;
    static constexpr std::int32_t MIN_STEP = -128;
    static constexpr std::int32_t MAX_STEP = 127;

    EntityChange(
        std::uint16_t id, std::uint8_t fields, std::int16_t x, std::int16_t y, std::uint8_t kind, std::uint8_t visual);

    static EntityChange created(const EntityState &entity);
    static std::optional<EntityChange> between(const EntityState &base, const EntityState &current);

    std::uint16_t id() const;
    std::uint8_t fields() const;
    bool isValid() const;
    std::optional<EntityState> resolve(const EntityState *base) const;

    void write(net::ByteWriter &writer) const;
    static EntityChange read(net::ByteReader &reader);

private:
    std::uint16_t m_id;
    std::uint8_t m_fields;
    std::int16_t m_x;
    std::int16_t m_y;
    std::uint8_t m_kind;
    std::uint8_t m_visual;
};

} // namespace engine::network
