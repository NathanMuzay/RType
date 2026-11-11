#include "SoundCue.hpp"

#include <utility>

namespace engine::ecs {

SoundCue::SoundCue(std::string sound) : m_sound(std::move(sound))
{
}

const std::string &SoundCue::sound() const
{
    return m_sound;
}

} // namespace engine::ecs
