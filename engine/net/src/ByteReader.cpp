#include "ByteReader.hpp"

#include <bit>

#include "NetError.hpp"

namespace engine::net {

ByteReader::ByteReader(std::span<const std::uint8_t> bytes) : m_bytes(bytes), m_offset(0)
{
}

std::uint8_t ByteReader::readU8()
{
    return static_cast<std::uint8_t>(readBigEndian(1));
}

std::uint16_t ByteReader::readU16()
{
    return static_cast<std::uint16_t>(readBigEndian(2));
}

std::uint32_t ByteReader::readU32()
{
    return readBigEndian(4);
}

std::int16_t ByteReader::readI16()
{
    return std::bit_cast<std::int16_t>(readU16());
}

std::string ByteReader::readString()
{
    const std::size_t length = readU8();
    if (length > remaining())
        throw NetError("ByteReader: truncated text");
    std::string text;
    text.reserve(length);
    for (std::size_t i = 0; i < length; ++i)
        text.push_back(static_cast<char>(m_bytes[m_offset++]));
    return text;
}

std::size_t ByteReader::remaining() const
{
    return m_bytes.size() - m_offset;
}

std::uint32_t ByteReader::readBigEndian(std::size_t byteCount)
{
    if (byteCount > remaining())
        throw NetError("ByteReader: truncated data");
    std::uint32_t value = 0;
    for (std::size_t i = 0; i < byteCount; ++i)
        value = (value << 8) | m_bytes[m_offset++];
    return value;
}

} // namespace engine::net
