#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Action.hpp"
#include "Clock.hpp"
#include "ComponentFactory.hpp"
#include "DataNode.hpp"
#include "Entity.hpp"
#include "FontManager.hpp"
#include "InputManager.hpp"
#include "LayerTable.hpp"
#include "Match.hpp"
#include "PlayerSettings.hpp"
#include "PluginManager.hpp"
#include "PrefabLibrary.hpp"
#include "Registry.hpp"
#include "SceneManager.hpp"
#include "SettingsStore.hpp"
#include "SfmlAudio.hpp"
#include "SfmlRenderer.hpp"
#include "SystemContext.hpp"
#include "SystemFactory.hpp"
#include "SystemManager.hpp"
#include "TextureManager.hpp"
#include "UiActionEvent.hpp"
#include "UiInput.hpp"
#include "UiStore.hpp"
#include "Window.hpp"
#include "WorldValueTable.hpp"

namespace engine::client {

class GameView
{
public:
    static constexpr const char *USER_SETTINGS_SUFFIX = "_user_settings.json";
    static constexpr const char *PLUGIN_FOLDER = "plugins";

    static constexpr float MAX_FRAME_SECONDS = 0.25f;
    static constexpr unsigned int MAX_KIND = 255;
    static constexpr float LOADING_BUDGET_SECONDS = 0.012f;
    static constexpr const char *LOADING_KEY = "loading.progress";

    GameView(const std::filesystem::path &assetsDirectory, const std::filesystem::path &dataDirectory,
        const std::string &game);

    bool isOpen() const;
    void closeWindow();
    void processEvents();
    bool isPressed(input::Action action) const;
    bool escapePressed() const;
    const PlayerSettings &settings() const;

    void switchScene(const std::string &name);
    const std::string &currentScene() const;
    std::string escapeAction() const;
    std::vector<ecs::UiActionEvent> takeUiActions();
    ecs::UiStore &uiStore();
    const ecs::UiStore &uiStore() const;
    void setFieldText(const std::string &field, const std::string &text);
    std::string message(const std::string &key) const;
    const scene::WorldValueTable &worldTable() const;
    const scene::WorldValueTable &playerTable() const;
    const scene::WorldValueTable &ruleTable() const;
    void playEventSound(std::uint8_t index);

    void beginSync();
    void syncEntity(std::uint16_t networkId, std::uint8_t kind, float x, float y, std::uint8_t visual);
    void endSync();
    void destroyEntity(std::uint16_t networkId);
    void setPhase(ecs::Match::Phase phase);
    void setPlayer(std::size_t slot, int lives, std::uint32_t score, std::uint16_t avatarId);
    void removePlayer(std::size_t slot);

    void frame();
    bool takeLoadingFinished();
    bool debugPressed() const;
    bool consolePressed() const;
    bool enterPressed() const;
    void focusField(const std::string &field);
    void focusAction(const std::string &action);
    void setMusicVolume(float volume);
    void setSoundVolume(float volume);
    void resetSettings();
    void reloadPlugins();
    void replayIntro();
    std::string anyKeyAction() const;
    std::vector<std::string> pluginList() const;
    const std::string &typedText() const;
    std::string secretCode() const;
    std::size_t entityCount() const;
    void close();

private:
    void loadResources(const std::filesystem::path &assetsDirectory);
    void loadStep();
    void preload(const std::string &name);
    void loadKinds();
    void clearRemoteWorld();
    ecs::Entity createRemote(const std::string &prefab, float x, float y);
    std::optional<ecs::Entity> playerEntity(std::size_t slot) const;
    static std::uint8_t parseKind(const std::string &text);
    static float verticalDirection(float previousY, float nextY);
    void applyVisual(const ecs::Entity &entity, std::uint8_t visual);
    static PlayerSettings loadUserSettings(const std::filesystem::path &file, const PlayerSettings &defaults);
    static scene::DataNode loadLinked(
        const std::filesystem::path &dataDirectory, const scene::DataNode &game, const std::string &key);

    scene::DataNode m_game;
    PlayerSettings m_defaults;
    PlayerSettings m_settings;
    render::Window m_window;
    render::TextureManager m_textures;
    render::FontManager m_fonts;
    render::SfmlRenderer m_renderer;
    audio::SfmlAudio m_audio;
    input::InputManager m_input;
    UiInput m_uiInput;
    ecs::UiStore m_uiStore;
    scene::WorldValueTable m_worldTable;
    scene::WorldValueTable m_playerTable;
    scene::WorldValueTable m_ruleTable;
    scene::WorldValueTable m_soundTable;
    ecs::Registry m_registry;
    scene::LayerTable m_layers;
    scene::ComponentFactory m_components;
    scene::PrefabLibrary m_prefabs;
    scene::SystemFactory m_systemFactory;
    scene::PluginManager m_plugins;
    scene::SystemContext m_context;
    SettingsStore m_settingsStore;
    ecs::SystemManager m_render;
    std::unique_ptr<SceneManager> m_scenes;
    Clock m_clock;
    std::optional<ecs::Entity> m_match;
    std::unordered_map<std::uint8_t, std::string> m_kindPrefabs;
    std::unordered_map<std::uint16_t, ecs::Entity> m_networkEntities;
    std::unordered_set<std::uint16_t> m_seen;
    std::vector<std::string> m_preload;
    std::size_t m_preloaded;
    bool m_loading;
    bool m_loadingFinished;
    Clock m_loadingClock;
    float m_loadingMinimum;
};

} // namespace engine::client
