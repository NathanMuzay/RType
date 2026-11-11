#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"

namespace engine::network {

class RoomInfo
{
public:
    static constexpr std::size_t FIXED_SIZE = 6;
    static constexpr std::uint8_t LOCKED_FLAG = 0x01;
    static constexpr std::uint8_t PLAYING_FLAG = 0x02;

    RoomInfo(
        std::uint16_t id, std::string name, std::uint8_t players, std::uint8_t capacity, bool locked, bool playing);

    std::uint16_t id() const;
    const std::string &name() const;
    std::uint8_t players() const;
    std::uint8_t capacity() const;
    bool locked() const;
    bool playing() const;

    void write(net::ByteWriter &writer) const;
    static RoomInfo read(net::ByteReader &reader);
    static std::size_t sizeFor(std::size_t nameLength);

private:
    std::uint16_t m_id;
    std::string m_name;
    std::uint8_t m_players;
    std::uint8_t m_capacity;
    bool m_locked;
    bool m_playing;
};

} // namespace engine::network
