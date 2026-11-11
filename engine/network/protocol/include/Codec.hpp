#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Packet.hpp"
#include "Protocol.hpp"

namespace engine::network {

class Codec
{
public:
    static constexpr std::size_t PAYLOAD_SIZE_OFFSET = 18;
    static constexpr std::uint8_t TO_CLIENT_BIT = 0x80;
    static constexpr std::uint32_t HALF_SEQUENCE_RANGE = 0x80000000u;

    explicit Codec(NetworkSettings settings);

    std::vector<std::uint8_t> encode(const Packet &packet) const;
    std::optional<Packet> decode(std::span<const std::uint8_t> bytes, Receiver receiver) const;
    static bool isNewer(std::uint32_t candidate, std::uint32_t previous);

private:
    bool isValid(const Packet &packet) const;
    static void writeBody(net::ByteWriter &writer, const Packet &packet);
    static Packet readBody(net::ByteReader &reader, MessageType type, std::uint32_t sequence, std::uint8_t version);

    template <class Message>
    static Packet readAs(net::ByteReader &reader, std::uint32_t sequence, std::uint8_t version);

    NetworkSettings m_settings;
};

} // namespace engine::network

#include "Codec.tpp"
