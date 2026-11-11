#include "SettingsStore.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <utility>

#include "DataWriter.hpp"
#include "EngineError.hpp"
#include "KeyBindings.hpp"

namespace engine::client {

SettingsStore::SettingsStore(ecs::UiStore &store, input::InputManager &input, audio::IAudio &audio,
    render::SfmlRenderer &renderer, scene::SystemContext &context, std::filesystem::path file)
    : m_store(store), m_input(input), m_audio(audio), m_renderer(renderer), m_context(context), m_file(std::move(file)),
      m_serverAddress(), m_values()
{
}

void SettingsStore::load(const PlayerSettings &settings)
{
    m_values.clear();
    m_serverAddress = settings.serverAddress();
    m_values["musicVolume"] = number(settings.musicVolume());
    m_values["soundVolume"] = number(settings.soundVolume());
    m_values["textScale"] = decimal(settings.textScale());
    m_values["deadzone"] = number(settings.deadzone());
    m_values["toggleFire"] = settings.toggleFire() ? ON : OFF;
    m_values["showPlayerLabels"] = settings.showPlayerLabels() ? ON : OFF;
    for (const auto &[name, action] : actions())
        m_values[KEY_PREFIX + std::string(name)] = settings.key(action);
    for (const auto &[name, action] : padActions())
        m_values[PAD_PREFIX + std::string(name)] = std::to_string(settings.padButton(action));
    publish();
}

void SettingsStore::update()
{
    if (m_store.value(CAPTURING_KEY) == YES)
        return;
    bool changed = false;
    for (auto &entry : m_values) {
        const std::string name = entry.first;
        const std::string wanted = m_store.value(PREFIX + name);
        if (wanted.empty() || wanted == entry.second)
            continue;
        if (apply(name, wanted))
            changed = true;
        m_store.setValue(PREFIX + name, m_values[name]);
    }
    if (changed)
        save();
}

void SettingsStore::reset(const PlayerSettings &defaults)
{
    load(defaults);
    const std::map<std::string, std::string> values = m_values;
    for (const auto &[name, value] : values)
        apply(name, value);
    publish();
    save();
}

void SettingsStore::publish()
{
    for (const auto &[name, value] : m_values)
        m_store.setValue(PREFIX + name, value);
}

bool SettingsStore::apply(const std::string &name, const std::string &value)
{
    if (name == "musicVolume") {
        const float volume = toFloat(value, 0.f, MAX_VOLUME);
        m_audio.setMusicVolume(volume);
        m_values[name] = number(volume);
    } else if (name == "soundVolume") {
        const float volume = toFloat(value, 0.f, MAX_VOLUME);
        m_audio.setSoundVolume(volume);
        m_values[name] = number(volume);
    } else if (name == "textScale") {
        const float scale = toFloat(value, MIN_TEXT_SCALE, MAX_TEXT_SCALE);
        m_renderer.setTextScale(scale);
        m_values[name] = decimal(scale);
    } else if (name == "deadzone") {
        const float deadzone = toFloat(value, 0.f, MAX_VOLUME);
        m_input.bindings().setDeadzone(deadzone);
        m_values[name] = number(deadzone);
    } else if (name == "toggleFire") {
        m_input.setToggle(input::Action::Fire, value == ON);
        m_values[name] = value == ON ? ON : OFF;
    } else if (name == "showPlayerLabels") {
        m_context.setFlag(name, value == ON);
        m_values[name] = value == ON ? ON : OFF;
    } else {
        for (const auto &[action, bound] : actions()) {
            if (name == KEY_PREFIX + std::string(action))
                return applyKey(bound, name, value);
        }
        for (const auto &[action, bound] : padActions()) {
            if (name == PAD_PREFIX + std::string(action))
                return applyPad(bound, name, value);
        }
        return false;
    }
    return true;
}

bool SettingsStore::applyKey(input::Action action, const std::string &name, const std::string &value)
{
    const auto key = keyName(value);
    if (!key)
        return false;
    const std::string previous = m_values[name];
    for (const auto &[other, bound] : actions()) {
        const std::string otherName = KEY_PREFIX + std::string(other);
        if (otherName != name && m_values[otherName] == *key) {
            m_values[otherName] = previous;
            m_input.bindings().bind(bound, input::KeyBindings::keyFromName(previous));
            m_store.setValue(PREFIX + otherName, previous);
        }
    }
    m_input.bindings().bind(action, input::KeyBindings::keyFromName(*key));
    m_values[name] = *key;
    return true;
}

bool SettingsStore::applyPad(input::Action action, const std::string &name, const std::string &value)
{
    const std::string digits = value.rfind(RAW_PREFIX, 0) == 0 ? value.substr(1) : value;
    if (!isNumber(digits))
        return false;
    const std::string previous = m_values[name];
    for (const auto &[other, bound] : padActions()) {
        const std::string otherName = PAD_PREFIX + std::string(other);
        if (otherName != name && m_values[otherName] == digits) {
            m_values[otherName] = previous;
            m_input.bindings().bindButton(bound, static_cast<unsigned int>(std::stoul(previous)));
            m_store.setValue(PREFIX + otherName, previous);
        }
    }
    m_input.bindings().bindButton(action, static_cast<unsigned int>(std::stoul(digits)));
    m_values[name] = digits;
    return true;
}

std::optional<std::string> SettingsStore::keyName(const std::string &value) const
{
    std::string name = value;
    if (value.rfind(RAW_PREFIX, 0) == 0) {
        const std::string digits = value.substr(1);
        if (!isNumber(digits))
            return std::nullopt;
        name = input::KeyBindings::nameOf(static_cast<sf::Keyboard::Scancode>(std::stoi(digits)));
    }
    if (name.empty())
        return std::nullopt;
    try {
        input::KeyBindings::keyFromName(name);
    } catch (const EngineError &) {
        return std::nullopt;
    }
    return name;
}

void SettingsStore::save() const
{
    scene::DataWriter writer;
    for (const auto &[name, action] : actions())
        writer.setString("controls", name, m_values.at(KEY_PREFIX + std::string(name)));
    for (const auto &[name, action] : padActions())
        writer.setInteger("gamepad", name, std::stoi(m_values.at(PAD_PREFIX + std::string(name))));
    writer.setInteger("gamepad", "deadzone", std::stoi(m_values.at("deadzone")));
    writer.setBool("", "toggleFire", m_values.at("toggleFire") == ON);
    writer.setBool("", "showPlayerLabels", m_values.at("showPlayerLabels") == ON);
    writer.setInteger("", "musicVolume", std::stoi(m_values.at("musicVolume")));
    writer.setInteger("", "soundVolume", std::stoi(m_values.at("soundVolume")));
    writer.setNumber("", "textScale", std::stof(m_values.at("textScale")));
    writer.setString("", "serverAddress", m_serverAddress);
    try {
        writer.save(m_file);
    } catch (const EngineError &error) {
        std::cerr << error.what() << std::endl;
    }
}

const std::array<SettingsStore::ActionName, SettingsStore::ACTION_COUNT> &SettingsStore::actions()
{
    static const std::array<ActionName, ACTION_COUNT> names = {{
        {"up", input::Action::Up},
        {"down", input::Action::Down},
        {"left", input::Action::Left},
        {"right", input::Action::Right},
        {"fire", input::Action::Fire},
        {"action", input::Action::Special},
    }};
    return names;
}

const std::array<SettingsStore::ActionName, SettingsStore::PAD_ACTION_COUNT> &SettingsStore::padActions()
{
    static const std::array<ActionName, PAD_ACTION_COUNT> names = {{
        {"fire", input::Action::Fire},
        {"action", input::Action::Special},
    }};
    return names;
}

std::string SettingsStore::number(float value)
{
    return std::to_string(static_cast<int>(std::lround(value)));
}

std::string SettingsStore::decimal(float value)
{
    const long tenths = std::lround(value * 10.f);
    return std::to_string(tenths / 10) + "." + std::to_string(tenths % 10);
}

float SettingsStore::toFloat(const std::string &text, float minimum, float maximum)
{
    return std::clamp(std::strtof(text.c_str(), nullptr), minimum, maximum);
}

bool SettingsStore::isNumber(const std::string &text)
{
    if (text.empty())
        return false;
    for (char character : text) {
        if (!std::isdigit(static_cast<unsigned char>(character)))
            return false;
    }
    return true;
}

} // namespace engine::client
