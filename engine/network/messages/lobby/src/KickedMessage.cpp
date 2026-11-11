#include "KickedMessage.hpp"

namespace engine::network {

bool KickedMessage::isValid(const NetworkSettings &) const
{
    return true;
}

void KickedMessage::write(net::ByteWriter &) const
{
}

KickedMessage KickedMessage::read(net::ByteReader &)
{
    return KickedMessage();
}

} // namespace engine::network
