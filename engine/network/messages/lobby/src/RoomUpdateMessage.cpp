#include "RoomUpdateMessage.hpp"

#include "TextRules.hpp"

#include <cstddef>
#include <utility>

namespace engine::network {

RoomUpdateMessage::RoomUpdateMessage(std::uint8_t ownerSlot, std::vector<bool> occupied, std::vector<std::string> names)
    : m_ownerSlot(ownerSlot), m_occupied(std::move(occupied)), m_names(std::move(names))
{
}

std::uint8_t RoomUpdateMessage::ownerSlot() const
{
    return m_ownerSlot;
}

const std::vector<bool> &RoomUpdateMessage::occupied() const
{
    return m_occupied;
}

const std::vector<std::string> &RoomUpdateMessage::names() const
{
    return m_names;
}

bool RoomUpdateMessage::isValid(const NetworkSettings &settings) const
{
    if (m_occupied.empty() || m_occupied.size() > settings.maxPlayers() || m_names.size() != m_occupied.size())
        return false;
    for (const std::string &name : m_names) {
        if (!TextRules::isValid(name, 0, settings.playerNameMaxLength()))
            return false;
    }
    return m_ownerSlot < m_occupied.size() && m_occupied[m_ownerSlot];
}

void RoomUpdateMessage::write(net::ByteWriter &writer) const
{
    writer.writeU8(m_ownerSlot);
    writer.writeU8(static_cast<std::uint8_t>(m_occupied.size()));
    for (std::size_t slot = 0; slot < m_occupied.size(); ++slot) {
        writer.writeU8(m_occupied[slot] ? 1 : 0);
        writer.writeString(m_names[slot]);
    }
}

RoomUpdateMessage RoomUpdateMessage::read(net::ByteReader &reader)
{
    const std::uint8_t ownerSlot = reader.readU8();
    const std::size_t slotCount = reader.readU8();
    std::vector<bool> occupied;
    std::vector<std::string> names;
    occupied.reserve(slotCount);
    names.reserve(slotCount);
    for (std::size_t i = 0; i < slotCount; ++i) {
        occupied.push_back(reader.readU8() != 0);
        names.push_back(reader.readString());
    }
    return RoomUpdateMessage(ownerSlot, std::move(occupied), std::move(names));
}

} // namespace engine::network
