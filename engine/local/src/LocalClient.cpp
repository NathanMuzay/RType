#include "LocalClient.hpp"

#include <vector>

#include "Action.hpp"
#include "PlaySoundMessage.hpp"
#include "Protocol.hpp"

namespace engine::local {

LocalClient::LocalClient(const std::filesystem::path &assetsDirectory, const std::filesystem::path &dataDirectory,
    const std::string &game, const network::NetworkSettings &settings, scripting::ScriptEngine &scripts)
    : m_view(assetsDirectory, dataDirectory, game), m_presenter(m_view), m_data(dataDirectory), m_game(game),
      m_settings(settings), m_scripts(scripts), m_simulation(), m_accumulator(0.f),
      m_tick(1.f / static_cast<float>(settings.tickRate())), m_settingsReturn()
{
}

void LocalClient::run()
{
    Clock::time_point last = Clock::now();
    while (m_view.isOpen()) {
        const Clock::time_point now = Clock::now();
        const float elapsed = std::chrono::duration<float>(now - last).count();
        last = now;
        m_view.processEvents();
        step(elapsed);
        m_view.frame();
        m_view.takeLoadingFinished();
        handleActions();
    }
    m_view.close();
}

void LocalClient::step(float elapsed)
{
    if (!m_simulation || paused())
        return;
    m_accumulator += elapsed < MAX_FRAME_SECONDS ? elapsed : MAX_FRAME_SECONDS;
    bool updated = false;
    while (m_accumulator >= m_tick) {
        m_simulation->setInput(SLOT, buttons());
        m_simulation->update(m_tick);
        m_accumulator -= m_tick;
        updated = true;
    }
    if (!updated)
        return;
    for (const network::Packet &event : m_simulation->takeEvents())
        if (event.holds<network::PlaySoundMessage>())
            m_view.playEventSound(event.as<network::PlaySoundMessage>().sound());
    m_presenter.apply(m_simulation->snapshot());
}

void LocalClient::handleActions()
{
    for (const ecs::UiActionEvent &event : m_view.takeUiActions())
        handleAction(event.action());
    if (m_view.uiStore().value(CAPTURING_KEY) == YES)
        return;
    if (m_view.escapePressed() && !m_view.escapeAction().empty())
        handleAction(m_view.escapeAction());
}

void LocalClient::handleAction(const std::string &action)
{
    if (action == "quit")
        m_view.closeWindow();
    else if (action == "play")
        start();
    else if (action == "pause")
        setPaused(!paused());
    else if (action == "resume")
        setPaused(false);
    else if (action == "settings")
        openSettings();
    else if (action == "back")
        closeSettings();
    else if (action == "settings_reset")
        m_view.resetSettings();
    else if (action.rfind("scene:", 0) == 0)
        stop(action.substr(std::string("scene:").size()));
}

void LocalClient::start()
{
    const std::vector<std::uint8_t> rules(m_settings.ruleCount(), 0);
    m_simulation = std::make_unique<simulation::Simulation>(m_data, m_game, m_settings, m_scripts, rules);
    m_simulation->addPlayer(SLOT);
    m_accumulator = 0.f;
    m_view.switchScene(GAME_SCENE);
    setPaused(false);
}

void LocalClient::stop(const std::string &scene)
{
    m_simulation.reset();
    m_view.switchScene(scene);
    setPaused(false);
}

void LocalClient::openSettings()
{
    m_settingsReturn = m_view.currentScene();
    m_view.switchScene(SETTINGS_SCENE);
}

void LocalClient::closeSettings()
{
    if (m_view.currentScene() != SETTINGS_SCENE || m_settingsReturn.empty())
        return;
    m_view.switchScene(m_settingsReturn);
    if (m_simulation)
        m_presenter.apply(m_simulation->snapshot());
}

void LocalClient::setPaused(bool paused)
{
    m_view.uiStore().setValue(PAUSE_KEY, paused ? YES : NO);
    if (paused)
        m_view.focusAction("resume");
}

bool LocalClient::paused() const
{
    return m_view.uiStore().value(PAUSE_KEY) == YES;
}

std::uint8_t LocalClient::buttons() const
{
    using input::Action;

    std::uint8_t bits = 0;
    if (m_view.isPressed(Action::Up))
        bits |= network::Protocol::BUTTON_UP;
    if (m_view.isPressed(Action::Down))
        bits |= network::Protocol::BUTTON_DOWN;
    if (m_view.isPressed(Action::Left))
        bits |= network::Protocol::BUTTON_LEFT;
    if (m_view.isPressed(Action::Right))
        bits |= network::Protocol::BUTTON_RIGHT;
    if (m_view.isPressed(Action::Fire))
        bits |= network::Protocol::BUTTON_FIRE;
    if (m_view.isPressed(Action::Special))
        bits |= network::Protocol::BUTTON_ACTION;
    return bits;
}

} // namespace engine::local
