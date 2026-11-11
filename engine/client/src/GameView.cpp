#include "GameView.hpp"

#include <algorithm>
#include <charconv>
#include <filesystem>
#include <iostream>
#include <system_error>

#include "Button.hpp"
#include "ClientError.hpp"
#include "ComponentRegistration.hpp"
#include "DeathEvent.hpp"
#include "Destroyed.hpp"
#include "EngineComponentLoaders.hpp"
#include "EngineError.hpp"
#include "EngineSystemBuilders.hpp"
#include "HitFlash.hpp"
#include "Lives.hpp"
#include "Persistent.hpp"
#include "Player.hpp"
#include "PlayerIndex.hpp"
#include "Protocol.hpp"
#include "Rotation.hpp"
#include "Score.hpp"
#include "TextField.hpp"
#include "TiltAnimation.hpp"
#include "Transform.hpp"
#include "UiFocus.hpp"

namespace engine::client {

GameView::GameView(
    const std::filesystem::path &assetsDirectory, const std::filesystem::path &dataDirectory, const std::string &game)
    : m_game(scene::DataNode::fromFile(dataDirectory / (game + "_game.json"))),
      m_defaults(loadLinked(dataDirectory, m_game, "settings")),
      m_settings(loadUserSettings(dataDirectory / (game + USER_SETTINGS_SUFFIX), m_defaults)),
      m_window(static_cast<unsigned int>(m_game.child("window").getInt("width")),
          static_cast<unsigned int>(m_game.child("window").getInt("height")), m_game.child("window").getString("title"),
          {m_game.child("world").getFloat("width"), m_game.child("world").getFloat("height")}),
      m_renderer(m_window, m_textures, m_fonts), m_prefabs(m_registry, m_components),
      m_plugins(m_systemFactory, dataDirectory.parent_path() / PLUGIN_FOLDER), m_context(m_registry, m_prefabs),
      m_settingsStore(
          m_uiStore, m_input, m_audio, m_renderer, m_context, dataDirectory / (game + USER_SETTINGS_SUFFIX)),
      m_preload(), m_preloaded(0), m_loading(false), m_loadingFinished(false), m_loadingClock(),
      m_loadingMinimum(m_game.getFloat("loadingMinSeconds", 0.f))
{
    ecs::registerEngineComponents(m_registry);
    m_layers.load(m_game.child("layers"));
    if (m_game.has("worldValues"))
        m_worldTable.load(m_game.child("worldValues"));
    if (m_game.has("playerValues"))
        m_playerTable.load(m_game.child("playerValues"));
    if (m_game.has("rules"))
        m_ruleTable.load(m_game.child("rules"));
    if (m_game.has("eventSounds"))
        m_soundTable.load(m_game.child("eventSounds"));
    scene::EngineComponentLoaders::registerAll(m_components, m_layers);
    scene::EngineSystemBuilders::registerAll(m_systemFactory);
    if (m_game.has("plugins"))
        m_plugins.load(m_game.child("plugins"));
    m_prefabs.load(loadLinked(dataDirectory, m_game, "clientPrefabs"));
    m_settings.applyControls(m_input);
    m_renderer.setTextScale(m_settings.textScale());
    loadResources(assetsDirectory);
    loadKinds();
    m_context.setRenderer(m_renderer);
    m_context.setAudio(m_audio);
    m_context.setUiInput(m_uiInput);
    m_context.setUiStore(m_uiStore);
    m_context.setFlag("showPlayerLabels", m_settings.showPlayerLabels());
    m_settingsStore.load(m_settings);
    m_systemFactory.build(m_context, m_game.child("renderSystems"), m_render);
    m_match = m_registry.spawn();
    m_registry.emplace<ecs::Match>(*m_match, 0.f);
    m_registry.emplace<ecs::Persistent>(*m_match);
    m_scenes = std::make_unique<SceneManager>(m_registry, m_prefabs, m_components, m_systemFactory, m_context, m_audio,
        Vector2(m_game.child("world").getFloat("width"), m_game.child("world").getFloat("height")),
        m_game.child("scenes"));
    if (m_game.has("loadingScene") && !m_preload.empty()) {
        m_loading = true;
        m_scenes->switchTo(m_game.getString("loadingScene"));
        return;
    }
    for (const std::string &name : m_preload)
        preload(name);
    m_scenes->switchTo(m_game.getString("startScene"));
}

bool GameView::isOpen() const
{
    return m_window.isOpen();
}

void GameView::closeWindow()
{
    m_window.close();
}

void GameView::processEvents()
{
    m_uiInput.beginFrame();
    m_window.processEvents(m_uiInput);
    m_input.update(m_window.hasFocus());
}

bool GameView::isPressed(input::Action action) const
{
    return m_input.isPressed(action);
}

bool GameView::escapePressed() const
{
    return m_uiInput.keyPressed(UiKey::Escape);
}

const PlayerSettings &GameView::settings() const
{
    return m_settings;
}

void GameView::switchScene(const std::string &name)
{
    clearRemoteWorld();
    m_scenes->switchTo(name);
}

const std::string &GameView::currentScene() const
{
    return m_scenes->current();
}

std::string GameView::escapeAction() const
{
    return m_scenes->escapeAction();
}

std::vector<ecs::UiActionEvent> GameView::takeUiActions()
{
    return m_context.uiActions().take();
}

ecs::UiStore &GameView::uiStore()
{
    return m_uiStore;
}

void GameView::setFieldText(const std::string &field, const std::string &text)
{
    auto &fields = m_registry.getComponents<ecs::TextField>();
    for (std::size_t i : m_registry.view<ecs::TextField>()) {
        if (fields.at(i).name() == field)
            fields.at(i).setContent(text);
    }
}

std::string GameView::message(const std::string &key) const
{
    if (!m_game.has("uiMessages") || !m_game.child("uiMessages").has(key))
        return key;
    return m_game.child("uiMessages").getString(key);
}

const scene::WorldValueTable &GameView::worldTable() const
{
    return m_worldTable;
}

void GameView::frame()
{
    const float dt = std::min(m_clock.restart(), MAX_FRAME_SECONDS);
    m_settingsStore.update();
    if (m_plugins.changed())
        reloadPlugins();
    if (m_loading)
        loadStep();
    m_scenes->update(dt);
    m_context.deaths().clear();
    m_render.update(dt);
}

void GameView::close()
{
    m_audio.stopMusic();
}

void GameView::loadResources(const std::filesystem::path &assetsDirectory)
{
    const scene::DataNode textures = m_game.child("textures");
    for (const std::string &name : textures.keys())
        m_textures.declare(name, (assetsDirectory / textures.getString(name)).string());
    if (m_game.has("fonts")) {
        const scene::DataNode fonts = m_game.child("fonts");
        for (const std::string &name : fonts.keys())
            m_fonts.declare(name, (assetsDirectory / fonts.getString(name)).string());
    }
    if (m_game.has("sounds")) {
        const scene::DataNode sounds = m_game.child("sounds");
        for (const std::string &name : sounds.keys())
            m_audio.declareSound(name, (assetsDirectory / sounds.getString(name)).string());
    }
    if (m_game.has("music")) {
        const scene::DataNode music = m_game.child("music");
        for (const std::string &name : music.keys())
            m_audio.declareMusic(name, (assetsDirectory / music.getString(name)).string());
    }
    if (m_game.has("preload")) {
        for (const scene::DataNode &name : m_game.child("preload").items())
            m_preload.push_back(name.asString());
    }
    m_audio.setMusicVolume(m_settings.musicVolume());
    m_audio.setSoundVolume(m_settings.soundVolume());
}

void GameView::loadKinds()
{
    const scene::DataNode kinds = m_game.child("kinds");
    for (const std::string &kind : kinds.keys()) {
        const std::string prefab = kinds.getString(kind);
        if (!m_prefabs.has(prefab))
            throw ClientError("GameView: unknown prefab " + prefab + " for entity kind " + kind);
        m_kindPrefabs.insert_or_assign(parseKind(kind), prefab);
    }
}

scene::DataNode GameView::loadLinked(
    const std::filesystem::path &dataDirectory, const scene::DataNode &game, const std::string &key)
{
    return scene::DataNode::fromFile(dataDirectory / game.child("files").getString(key));
}

const scene::WorldValueTable &GameView::playerTable() const
{
    return m_playerTable;
}

const scene::WorldValueTable &GameView::ruleTable() const
{
    return m_ruleTable;
}

void GameView::playEventSound(std::uint8_t index)
{
    if (index < m_soundTable.size())
        m_audio.playSound(m_soundTable.name(index));
}

bool GameView::takeLoadingFinished()
{
    const bool finished = m_loadingFinished;
    m_loadingFinished = false;
    return finished;
}

bool GameView::debugPressed() const
{
    return m_uiInput.keyPressed(UiKey::Debug);
}

void GameView::replayIntro()
{
    if (!m_game.has("loadingScene")) {
        m_scenes->switchTo(m_game.getString("startScene"));
        return;
    }
    m_loading = true;
    m_loadingClock.restart();
    m_scenes->switchTo(m_game.getString("loadingScene"));
}

std::string GameView::anyKeyAction() const
{
    if (!m_uiInput.rawKey())
        return {};
    const scene::DataNode scene = m_game.child("scenes").child(m_scenes->current());
    if (!scene.has("anyKey"))
        return {};
    const scene::DataNode rule = scene.child("anyKey");
    if (m_uiStore.value(rule.getString("key")) != rule.getString("value"))
        return {};
    return rule.getString("action");
}

const std::string &GameView::typedText() const
{
    return m_uiInput.typedText();
}

std::string GameView::secretCode() const
{
    return m_game.has("secretCode") ? m_game.getString("secretCode") : std::string();
}

std::size_t GameView::entityCount() const
{
    return m_registry.view<ecs::Transform>().size();
}

void GameView::loadStep()
{
    Clock budget;
    while (m_preloaded < m_preload.size() && budget.elapsed() < LOADING_BUDGET_SECONDS)
        preload(m_preload[m_preloaded++]);
    m_uiStore.setValue(
        LOADING_KEY, std::to_string(static_cast<float>(m_preloaded) / static_cast<float>(m_preload.size())));
    if (m_preloaded < m_preload.size() || m_loadingClock.elapsed() < m_loadingMinimum)
        return;
    m_loading = false;
    m_loadingFinished = true;
    m_scenes->switchTo(m_game.getString("startScene"));
}

void GameView::preload(const std::string &name)
{
    if (m_textures.declared(name))
        m_textures.preload(name);
    else if (m_audio.hasSound(name))
        m_audio.preloadSound(name);
    else
        throw ClientError("GameView: cannot preload unknown resource " + name);
}

bool GameView::consolePressed() const
{
    return m_uiInput.keyPressed(UiKey::Console);
}

bool GameView::enterPressed() const
{
    return m_uiInput.keyPressed(UiKey::Enter);
}

void GameView::focusField(const std::string &field)
{
    auto &fields = m_registry.getComponents<ecs::TextField>();
    for (std::size_t i : m_registry.view<ecs::TextField>())
        fields.at(i).setActive(!field.empty() && fields.at(i).name() == field);
}

void GameView::setMusicVolume(float volume)
{
    m_audio.setMusicVolume(volume);
}

void GameView::setSoundVolume(float volume)
{
    m_audio.setSoundVolume(volume);
}

void GameView::reloadPlugins()
{
    m_scenes->clearSystems();
    m_render = ecs::SystemManager();
    try {
        m_plugins.reload();
    } catch (const EngineError &error) {
        std::cerr << "[plugins] " << error.what() << std::endl;
    }
    m_systemFactory.build(m_context, m_game.child("renderSystems"), m_render);
    m_scenes->rebuildSystems();
}

std::vector<std::string> GameView::pluginList() const
{
    return m_plugins.describe();
}

void GameView::resetSettings()
{
    m_settingsStore.reset(m_defaults);
}

PlayerSettings GameView::loadUserSettings(const std::filesystem::path &file, const PlayerSettings &defaults)
{
    if (!std::filesystem::exists(file))
        return defaults;
    try {
        return PlayerSettings(scene::DataNode::fromFile(file));
    } catch (const EngineError &error) {
        std::cerr << error.what() << ", using the default settings" << std::endl;
        return defaults;
    }
}

void GameView::focusAction(const std::string &action)
{
    const auto &buttons = m_registry.getComponents<ecs::Button>();
    for (std::size_t i : m_registry.view<ecs::Button>()) {
        if (buttons.at(i).action() == action)
            ecs::UiFocus::hover(m_registry, m_registry.entityAt(i));
    }
}

const ecs::UiStore &GameView::uiStore() const
{
    return m_uiStore;
}

} // namespace engine::client
