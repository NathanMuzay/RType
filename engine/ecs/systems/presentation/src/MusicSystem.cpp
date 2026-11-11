#include "MusicSystem.hpp"

#include <cstddef>

#include "MusicWhen.hpp"

namespace engine::ecs {

MusicSystem::MusicSystem(Registry &registry, const UiStore &store, audio::IAudio &audio)
    : m_registry(registry), m_store(store), m_audio(audio), m_current()
{
}

void MusicSystem::update(float)
{
    const auto &rules = m_registry.getComponents<MusicWhen>();
    for (std::size_t i : m_registry.view<MusicWhen>()) {
        const MusicWhen &rule = rules.at(i);
        if (m_store.value(rule.key()) != rule.value())
            continue;
        if (rule.music() != m_current) {
            m_current = rule.music();
            m_audio.playMusic(m_current);
        }
        return;
    }
}

} // namespace engine::ecs
