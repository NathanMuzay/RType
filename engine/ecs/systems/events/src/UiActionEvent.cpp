#include "UiActionEvent.hpp"

#include <utility>

namespace engine::ecs {

UiActionEvent::UiActionEvent(std::string action) : m_action(std::move(action))
{
}

const std::string &UiActionEvent::action() const
{
    return m_action;
}

} // namespace engine::ecs
