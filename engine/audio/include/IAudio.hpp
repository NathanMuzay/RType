#pragma once

#include <string>

namespace engine::audio {

class IAudio
{
public:
    virtual ~IAudio() = default;

    virtual void playSound(const std::string &name) = 0;
    virtual void playMusic(const std::string &name) = 0;
    virtual void stopMusic() = 0;
    virtual void setSoundVolume(float volume) = 0;
    virtual void setMusicVolume(float volume) = 0;
};

} // namespace engine::audio
