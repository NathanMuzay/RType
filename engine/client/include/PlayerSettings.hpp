#pragma once

#include <string>

#include "DataNode.hpp"
#include "InputManager.hpp"

namespace engine::client {

class PlayerSettings
{
public:
    static constexpr float DEFAULT_MUSIC_VOLUME = 40.f;
    static constexpr float DEFAULT_SOUND_VOLUME = 70.f;
    static constexpr const char *DEFAULT_SERVER_ADDRESS = "127.0.0.1";
    static constexpr const char *DEFAULT_ACTION_KEY = "LShift";
    static constexpr int DEFAULT_PAD_FIRE = 0;
    static constexpr int DEFAULT_PAD_ACTION = 1;

    explicit PlayerSettings(const scene::DataNode &settings);

    void applyControls(input::InputManager &input) const;
    bool showPlayerLabels() const;
    float musicVolume() const;
    float soundVolume() const;
    float textScale() const;
    const std::string &serverAddress() const;
    const std::string &key(input::Action action) const;
    unsigned int padButton(input::Action action) const;
    float deadzone() const;
    bool toggleFire() const;

private:
    static int padSetting(const scene::DataNode &settings, const std::string &key, int fallback);

    std::string m_up;
    std::string m_down;
    std::string m_left;
    std::string m_right;
    std::string m_fire;
    std::string m_action;
    unsigned int m_padFire;
    unsigned int m_padAction;
    float m_padDeadzone;
    bool m_toggleFire;
    bool m_showPlayerLabels;
    float m_musicVolume;
    float m_soundVolume;
    float m_textScale;
    std::string m_serverAddress;
};

} // namespace engine::client
