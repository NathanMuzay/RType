#include "RoomClosedMessage.hpp"

namespace engine::network {

bool RoomClosedMessage::isValid(const NetworkSettings &) const
{
    return true;
}

void RoomClosedMessage::write(net::ByteWriter &) const
{
}

RoomClosedMessage RoomClosedMessage::read(net::ByteReader &)
{
    return RoomClosedMessage();
}

} // namespace engine::network
