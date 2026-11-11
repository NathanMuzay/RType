#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <utility>

#include "IAudio.hpp"
#include "InputManager.hpp"
#include "PlayerSettings.hpp"
#include "SfmlRenderer.hpp"
#include "SystemContext.hpp"
#include "UiStore.hpp"

namespace engine::client {

class SettingsStore
{
public:
    static constexpr const char *PREFIX = "setting.";
    static constexpr const char *ON = "ON";
    static constexpr const char *OFF = "OFF";
    static constexpr const char *RAW_PREFIX = "#";
    static constexpr const char *CAPTURING_KEY = "settings.capturing";
    static constexpr const char *YES = "yes";
    static constexpr const char *KEY_PREFIX = "key.";
    static constexpr const char *PAD_PREFIX = "pad.";
    static constexpr float MAX_VOLUME = 100.f;
    static constexpr float MIN_TEXT_SCALE = 0.5f;
    static constexpr float MAX_TEXT_SCALE = 3.f;
    static constexpr std::size_t ACTION_COUNT = 6;
    static constexpr std::size_t PAD_ACTION_COUNT = 2;

    SettingsStore(ecs::UiStore &store, input::InputManager &input, audio::IAudio &audio, render::SfmlRenderer &renderer,
        scene::SystemContext &context, std::filesystem::path file);

    void load(const PlayerSettings &settings);
    void update();
    void reset(const PlayerSettings &defaults);

private:
    using ActionName = std::pair<const char *, input::Action>;

    static const std::array<ActionName, ACTION_COUNT> &actions();
    static const std::array<ActionName, PAD_ACTION_COUNT> &padActions();
    static std::string number(float value);
    static std::string decimal(float value);
    static float toFloat(const std::string &text, float minimum, float maximum);
    static bool isNumber(const std::string &text);

    void publish();
    bool apply(const std::string &name, const std::string &value);
    bool applyKey(input::Action action, const std::string &name, const std::string &value);
    bool applyPad(input::Action action, const std::string &name, const std::string &value);
    std::optional<std::string> keyName(const std::string &value) const;
    void save() const;

    ecs::UiStore &m_store;
    input::InputManager &m_input;
    audio::IAudio &m_audio;
    render::SfmlRenderer &m_renderer;
    scene::SystemContext &m_context;
    std::filesystem::path m_file;
    std::string m_serverAddress;
    std::map<std::string, std::string> m_values;
};

} // namespace engine::client
