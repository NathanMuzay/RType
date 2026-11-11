#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"

namespace engine::network {

class PlayerStatus
{
public:
    static constexpr std::size_t SIZE = 10;
    static constexpr std::size_t VALUE_SIZE = 2;

    PlayerStatus();
    PlayerStatus(std::uint8_t state, std::uint8_t lives, std::uint8_t health, std::uint32_t score,
        std::uint16_t avatarId, std::vector<std::uint16_t> values);

    std::uint8_t state() const;
    std::uint8_t lives() const;
    std::uint8_t health() const;
    std::uint32_t score() const;
    std::uint16_t avatarId() const;
    const std::vector<std::uint16_t> &values() const;
    static std::size_t sizeFor(std::size_t values);

    bool operator==(const PlayerStatus &other) const;

    void write(engine::net::ByteWriter &writer) const;
    static PlayerStatus read(engine::net::ByteReader &reader);

private:
    std::uint8_t m_state;
    std::uint8_t m_lives;
    std::uint8_t m_health;
    std::uint32_t m_score;
    std::uint16_t m_avatarId;
    std::vector<std::uint16_t> m_values;
};

} // namespace engine::network
