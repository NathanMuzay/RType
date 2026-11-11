#pragma once

#include <string>

#include "IAudio.hpp"
#include "ISystem.hpp"
#include "Registry.hpp"
#include "UiStore.hpp"

namespace engine::ecs {

class MusicSystem : public ISystem
{
public:
    MusicSystem(Registry &registry, const UiStore &store, audio::IAudio &audio);

    void update(float dt) override;

private:
    Registry &m_registry;
    const UiStore &m_store;
    audio::IAudio &m_audio;
    std::string m_current;
};

} // namespace engine::ecs
