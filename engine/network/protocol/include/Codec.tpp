#pragma once

namespace engine::network {

template <class Message>
Packet Codec::readAs(net::ByteReader &reader, std::uint32_t sequence, std::uint8_t version)
{
    return Packet(Message::read(reader), sequence, version);
}

} // namespace engine::network
