#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "Packet.hpp"
#include "SnapshotMessage.hpp"

namespace engine::network {

class IGameSession
{
public:
    virtual ~IGameSession() = default;

    virtual void addPlayer(std::size_t slot) = 0;
    virtual void removePlayer(std::size_t slot) = 0;
    virtual void setInput(std::size_t slot, std::uint8_t buttons) = 0;
    virtual void command(std::size_t slot, const std::string &text) = 0;
    virtual void update(float dt) = 0;
    virtual std::uint32_t tick() const = 0;
    virtual SnapshotMessage snapshot() const = 0;
    virtual std::vector<Packet> takeEvents() = 0;
    virtual bool finished() const = 0;
};

} // namespace engine::network
