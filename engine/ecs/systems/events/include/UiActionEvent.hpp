#pragma once

#include <string>

namespace engine::ecs {

class UiActionEvent
{
public:
    explicit UiActionEvent(std::string action);

    const std::string &action() const;

private:
    std::string m_action;
};

} // namespace engine::ecs
