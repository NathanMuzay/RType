#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

#include "GameView.hpp"
#include "NetworkSettings.hpp"
#include "ScriptEngine.hpp"
#include "Simulation.hpp"
#include "SnapshotPresenter.hpp"

namespace engine::local {

class LocalClient
{
public:
    static constexpr const char *GAME_SCENE = "game";
    static constexpr const char *SETTINGS_SCENE = "settings";
    static constexpr const char *CAPTURING_KEY = "settings.capturing";
    static constexpr const char *PAUSE_KEY = "pause.visible";
    static constexpr const char *YES = "yes";
    static constexpr const char *NO = "no";
    static constexpr std::size_t SLOT = 0;
    static constexpr float MAX_FRAME_SECONDS = 0.25f;

    LocalClient(const std::filesystem::path &assetsDirectory, const std::filesystem::path &dataDirectory,
        const std::string &game, const network::NetworkSettings &settings, scripting::ScriptEngine &scripts);

    void run();

private:
    using Clock = std::chrono::steady_clock;

    void handleActions();
    void handleAction(const std::string &action);
    void start();
    void stop(const std::string &scene);
    void step(float elapsed);
    void setPaused(bool paused);
    void openSettings();
    void closeSettings();
    bool paused() const;
    std::uint8_t buttons() const;

    client::GameView m_view;
    client::SnapshotPresenter m_presenter;
    std::filesystem::path m_data;
    std::string m_game;
    network::NetworkSettings m_settings;
    scripting::ScriptEngine &m_scripts;
    std::unique_ptr<simulation::Simulation> m_simulation;
    float m_accumulator;
    float m_tick;
    std::string m_settingsReturn;
};

} // namespace engine::local
