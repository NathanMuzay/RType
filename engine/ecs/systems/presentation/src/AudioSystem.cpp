#include "AudioSystem.hpp"

#include "SoundCue.hpp"

namespace engine::ecs {

AudioSystem::AudioSystem(Registry &registry, audio::IAudio &audio) : m_registry(registry), m_audio(audio)
{
}

void AudioSystem::update(float)
{
    for (std::size_t i : m_registry.view<SoundCue>()) {
        const Entity entity = m_registry.entityAt(i);
        m_audio.playSound(m_registry.get<SoundCue>(entity).sound());
        m_registry.remove<SoundCue>(entity);
    }
}

} // namespace engine::ecs
