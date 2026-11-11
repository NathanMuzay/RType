#pragma once

#include <string>

#include "IComponent.hpp"

namespace engine::ecs {

class SoundCue : public IComponent
{
public:
    explicit SoundCue(std::string sound);

    const std::string &sound() const;

private:
    std::string m_sound;
};

} // namespace engine::ecs
