#include "PlaySoundMessage.hpp"

namespace engine::network {

PlaySoundMessage::PlaySoundMessage(std::uint8_t sound) : m_sound(sound)
{
}

std::uint8_t PlaySoundMessage::sound() const
{
    return m_sound;
}

bool PlaySoundMessage::isValid(const NetworkSettings &settings) const
{
    return m_sound < settings.eventSoundCount();
}

void PlaySoundMessage::write(net::ByteWriter &writer) const
{
    writer.writeU8(m_sound);
}

PlaySoundMessage PlaySoundMessage::read(net::ByteReader &reader)
{
    const std::uint8_t sound = reader.readU8();
    return PlaySoundMessage(sound);
}

} // namespace engine::network
