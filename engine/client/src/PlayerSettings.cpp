#include "PlayerSettings.hpp"

#include "KeyBindings.hpp"

namespace engine::client {

PlayerSettings::PlayerSettings(const scene::DataNode &settings)
    : m_up(settings.child("controls").getString("up")), m_down(settings.child("controls").getString("down")),
      m_left(settings.child("controls").getString("left")), m_right(settings.child("controls").getString("right")),
      m_fire(settings.child("controls").getString("fire")),
      m_action(settings.child("controls").has("action") ? settings.child("controls").getString("action")
                                                        : DEFAULT_ACTION_KEY),
      m_padFire(static_cast<unsigned int>(padSetting(settings, "fire", DEFAULT_PAD_FIRE))),
      m_padAction(static_cast<unsigned int>(padSetting(settings, "action", DEFAULT_PAD_ACTION))),
      m_padDeadzone(settings.has("gamepad")
              ? settings.child("gamepad").getFloat("deadzone", input::KeyBindings::DEFAULT_DEADZONE)
              : input::KeyBindings::DEFAULT_DEADZONE),
      m_toggleFire(settings.getBool("toggleFire", false)),
      m_showPlayerLabels(settings.getBool("showPlayerLabels", true)),
      m_musicVolume(settings.getFloat("musicVolume", DEFAULT_MUSIC_VOLUME)),
      m_soundVolume(settings.getFloat("soundVolume", DEFAULT_SOUND_VOLUME)),
      m_textScale(settings.getFloat("textScale", 1.f)),
      m_serverAddress(settings.has("serverAddress") ? settings.getString("serverAddress") : DEFAULT_SERVER_ADDRESS)
{
}

const std::string &PlayerSettings::serverAddress() const
{
    return m_serverAddress;
}

void PlayerSettings::applyControls(input::InputManager &input) const
{
    using input::Action;
    using input::KeyBindings;

    input.bindings().bind(Action::Up, KeyBindings::keyFromName(m_up));
    input.bindings().bind(Action::Down, KeyBindings::keyFromName(m_down));
    input.bindings().bind(Action::Left, KeyBindings::keyFromName(m_left));
    input.bindings().bind(Action::Right, KeyBindings::keyFromName(m_right));
    input.bindings().bind(Action::Fire, KeyBindings::keyFromName(m_fire));
    input.bindings().bind(Action::Special, KeyBindings::keyFromName(m_action));
    input.bindings().bindButton(Action::Fire, m_padFire);
    input.bindings().bindButton(Action::Special, m_padAction);
    input.bindings().setDeadzone(m_padDeadzone);
    input.setToggle(Action::Fire, m_toggleFire);
}

bool PlayerSettings::showPlayerLabels() const
{
    return m_showPlayerLabels;
}

float PlayerSettings::musicVolume() const
{
    return m_musicVolume;
}

float PlayerSettings::soundVolume() const
{
    return m_soundVolume;
}

float PlayerSettings::textScale() const
{
    return m_textScale;
}

int PlayerSettings::padSetting(const scene::DataNode &settings, const std::string &key, int fallback)
{
    if (!settings.has("gamepad"))
        return fallback;
    return settings.child("gamepad").getInt(key, fallback);
}

const std::string &PlayerSettings::key(input::Action action) const
{
    switch (action) {
    case input::Action::Up:
        return m_up;
    case input::Action::Down:
        return m_down;
    case input::Action::Left:
        return m_left;
    case input::Action::Right:
        return m_right;
    case input::Action::Fire:
        return m_fire;
    case input::Action::Special:
        break;
    }
    return m_action;
}

unsigned int PlayerSettings::padButton(input::Action action) const
{
    return action == input::Action::Special ? m_padAction : m_padFire;
}

float PlayerSettings::deadzone() const
{
    return m_padDeadzone;
}

bool PlayerSettings::toggleFire() const
{
    return m_toggleFire;
}

} // namespace engine::client
