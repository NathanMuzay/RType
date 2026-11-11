#include "ByteWriter.hpp"

#include <utility>

#include "NetError.hpp"

namespace engine::net {

ByteWriter::ByteWriter(std::size_t maxSize) : m_maxSize(maxSize)
{
    m_bytes.reserve(maxSize);
}

void ByteWriter::writeU8(std::uint8_t value)
{
    writeBigEndian(value, 1);
}

void ByteWriter::writeU16(std::uint16_t value)
{
    writeBigEndian(value, 2);
}

void ByteWriter::writeU32(std::uint32_t value)
{
    writeBigEndian(value, 4);
}

void ByteWriter::writeI16(std::int16_t value)
{
    writeU16(static_cast<std::uint16_t>(value));
}

void ByteWriter::writeString(const std::string &text)
{
    if (text.size() > MAX_STRING_LENGTH)
        throw NetError("ByteWriter: text is too long");
    writeU8(static_cast<std::uint8_t>(text.size()));
    for (char character : text)
        writeU8(static_cast<std::uint8_t>(character));
}

void ByteWriter::patchU16(std::size_t offset, std::uint16_t value)
{
    if (offset + 2 > m_bytes.size())
        throw NetError("ByteWriter: patch outside the written bytes");
    m_bytes[offset] = static_cast<std::uint8_t>(value >> 8);
    m_bytes[offset + 1] = static_cast<std::uint8_t>(value);
}

std::size_t ByteWriter::size() const
{
    return m_bytes.size();
}

std::vector<std::uint8_t> ByteWriter::release()
{
    return std::move(m_bytes);
}

void ByteWriter::writeBigEndian(std::uint32_t value, std::size_t byteCount)
{
    if (m_bytes.size() + byteCount > m_maxSize)
        throw NetError("ByteWriter: packet exceeds its maximum size");
    for (std::size_t i = byteCount; i > 0; --i)
        m_bytes.push_back(static_cast<std::uint8_t>(value >> ((i - 1) * 8)));
}

} // namespace engine::net
