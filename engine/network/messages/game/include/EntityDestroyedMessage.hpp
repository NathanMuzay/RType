#pragma once

#include <cstdint>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class EntityDestroyedMessage
{
public:
    static constexpr MessageType TYPE = MessageType::EntityDestroyed;

    explicit EntityDestroyedMessage(std::uint16_t entityId);

    std::uint16_t entityId() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(engine::net::ByteWriter &writer) const;
    static EntityDestroyedMessage read(engine::net::ByteReader &reader);

private:
    std::uint16_t m_entityId;
};

} // namespace engine::network
