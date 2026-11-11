#include "KeyBind.hpp"

#include <utility>

namespace engine::ecs {

KeyBind::KeyBind(std::string action, std::string device, std::string prompt)
    : m_action(std::move(action)), m_device(std::move(device)), m_prompt(std::move(prompt)), m_waiting(false),
      m_armed(false), m_previous()
{
}

const std::string &KeyBind::action() const
{
    return m_action;
}

bool KeyBind::gamepad() const
{
    return m_device == GAMEPAD;
}

const std::string &KeyBind::prompt() const
{
    return m_prompt;
}

std::string KeyBind::storeKey() const
{
    return (gamepad() ? PAD_PREFIX : KEY_PREFIX) + m_action;
}

bool KeyBind::waiting() const
{
    return m_waiting;
}

bool KeyBind::armed() const
{
    return m_armed;
}

const std::string &KeyBind::previous() const
{
    return m_previous;
}

void KeyBind::startWaiting(std::string previous)
{
    m_waiting = true;
    m_armed = false;
    m_previous = std::move(previous);
}

void KeyBind::arm()
{
    m_armed = true;
}

void KeyBind::stopWaiting()
{
    m_waiting = false;
    m_armed = false;
}

} // namespace engine::ecs
