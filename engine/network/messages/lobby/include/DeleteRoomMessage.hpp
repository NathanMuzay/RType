#pragma once

#include <cstdint>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class DeleteRoomMessage
{
public:
    static constexpr MessageType TYPE = MessageType::DeleteRoom;

    explicit DeleteRoomMessage(std::uint32_t token);

    std::uint32_t token() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static DeleteRoomMessage read(net::ByteReader &reader);

private:
    std::uint32_t m_token;
};

} // namespace engine::network
