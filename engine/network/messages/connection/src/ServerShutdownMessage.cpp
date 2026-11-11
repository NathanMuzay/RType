#include "ServerShutdownMessage.hpp"

namespace engine::network {

bool ServerShutdownMessage::isValid(const NetworkSettings &) const
{
    return true;
}

void ServerShutdownMessage::write(engine::net::ByteWriter &) const
{
}

ServerShutdownMessage ServerShutdownMessage::read(engine::net::ByteReader &)
{
    return ServerShutdownMessage();
}

} // namespace engine::network
