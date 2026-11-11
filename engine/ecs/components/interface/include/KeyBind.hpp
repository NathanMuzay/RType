#pragma once

#include <string>

#include "IComponent.hpp"

namespace engine::ecs {

class KeyBind : public IComponent
{
public:
    static constexpr const char *KEYBOARD = "keyboard";
    static constexpr const char *GAMEPAD = "gamepad";
    static constexpr const char *KEY_PREFIX = "setting.key.";
    static constexpr const char *PAD_PREFIX = "setting.pad.";
    static constexpr const char *DEFAULT_PROMPT = "...";

    KeyBind(std::string action, std::string device, std::string prompt);

    const std::string &action() const;
    bool gamepad() const;
    const std::string &prompt() const;
    std::string storeKey() const;
    bool waiting() const;
    bool armed() const;
    const std::string &previous() const;
    void startWaiting(std::string previous);
    void arm();
    void stopWaiting();

private:
    std::string m_action;
    std::string m_device;
    std::string m_prompt;
    bool m_waiting;
    bool m_armed;
    std::string m_previous;
};

} // namespace engine::ecs
