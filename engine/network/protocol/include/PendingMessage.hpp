#pragma once

#include <chrono>

#include "Packet.hpp"

namespace engine::network {

class PendingMessage
{
public:
    using Time = std::chrono::steady_clock::time_point;

    PendingMessage(Packet packet, Time sentAt);

    const Packet &packet() const;
    Time sentAt() const;

private:
    Packet m_packet;
    Time m_sentAt;
};

} // namespace engine::network
