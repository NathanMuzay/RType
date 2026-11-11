#pragma once

#include <vector>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"
#include "RoomInfo.hpp"

namespace engine::network {

class RoomListMessage
{
public:
    static constexpr MessageType TYPE = MessageType::RoomList;

    explicit RoomListMessage(std::vector<RoomInfo> rooms);

    const std::vector<RoomInfo> &rooms() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static RoomListMessage read(net::ByteReader &reader);

private:
    std::vector<RoomInfo> m_rooms;
};

} // namespace engine::network
