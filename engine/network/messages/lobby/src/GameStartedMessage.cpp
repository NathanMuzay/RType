#include "GameStartedMessage.hpp"

namespace engine::network {

bool GameStartedMessage::isValid(const NetworkSettings &) const
{
    return true;
}

void GameStartedMessage::write(net::ByteWriter &) const
{
}

GameStartedMessage GameStartedMessage::read(net::ByteReader &)
{
    return GameStartedMessage();
}

} // namespace engine::network
