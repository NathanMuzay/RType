#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

namespace engine::net {

class ByteReader
{
public:
    explicit ByteReader(std::span<const std::uint8_t> bytes);

    std::uint8_t readU8();
    std::uint16_t readU16();
    std::uint32_t readU32();
    std::int16_t readI16();
    std::string readString();

    std::size_t remaining() const;

private:
    std::uint32_t readBigEndian(std::size_t byteCount);

    std::span<const std::uint8_t> m_bytes;
    std::size_t m_offset;
};

} // namespace engine::net
