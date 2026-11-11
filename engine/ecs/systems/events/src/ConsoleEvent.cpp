#include "ConsoleEvent.hpp"

#include <utility>

namespace engine::ecs {

ConsoleEvent::ConsoleEvent(std::size_t slot, std::string text) : m_slot(slot), m_text(std::move(text))
{
}

std::size_t ConsoleEvent::slot() const
{
    return m_slot;
}

const std::string &ConsoleEvent::text() const
{
    return m_text;
}

} // namespace engine::ecs
