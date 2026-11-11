#pragma once

#include <chrono>
#include <cstddef>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include "IAudio.hpp"

namespace engine::audio {

class SfmlAudio : public IAudio
{
public:
    static constexpr std::size_t MAX_PLAYING_SOUNDS = 32;
    static constexpr float MAX_VOLUME = 100.f;
    static constexpr std::chrono::milliseconds REPEAT_GAP{40};

    void declareSound(const std::string &name, const std::string &path);
    void declareMusic(const std::string &name, const std::string &path);
    bool hasSound(const std::string &name) const;
    void preloadSound(const std::string &name);

    void playSound(const std::string &name) override;
    void playMusic(const std::string &name) override;
    void stopMusic() override;
    void setSoundVolume(float volume) override;
    void setMusicVolume(float volume) override;

private:
    void decode(const std::string &name, const std::string &path);

    std::unordered_map<std::string, sf::SoundBuffer> m_buffers;
    std::unordered_map<std::string, std::string> m_soundPaths;
    std::unordered_map<std::string, std::string> m_musicPaths;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> m_lastPlayed;
    std::list<sf::Sound> m_playing;
    std::unique_ptr<sf::Music> m_music;
    float m_soundVolume = MAX_VOLUME;
    float m_musicVolume = MAX_VOLUME;
};

} // namespace engine::audio
