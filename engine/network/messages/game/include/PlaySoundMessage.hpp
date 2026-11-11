#pragma once

#include <cstdint>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class PlaySoundMessage
{
public:
    static constexpr MessageType TYPE = MessageType::PlaySound;

    explicit PlaySoundMessage(std::uint8_t sound);

    std::uint8_t sound() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static PlaySoundMessage read(net::ByteReader &reader);

private:
    std::uint8_t m_sound;
};

} // namespace engine::network
