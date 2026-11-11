#pragma once

#include "IAudio.hpp"
#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class AudioSystem : public ISystem
{
public:
    AudioSystem(Registry &registry, audio::IAudio &audio);

    void update(float dt) override;

private:
    Registry &m_registry;
    audio::IAudio &m_audio;
};

} // namespace engine::ecs
