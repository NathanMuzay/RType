#include "StatusText.hpp"

#include <utility>

namespace engine::ecs {

StatusText::StatusText(std::string format, std::optional<std::size_t> slot, std::optional<Match::Phase> visibleIn)
    : m_format(std::move(format)), m_slot(slot), m_visibleIn(visibleIn)
{
}

const std::string &StatusText::format() const
{
    return m_format;
}

std::optional<std::size_t> StatusText::slot() const
{
    return m_slot;
}

bool StatusText::visibleIn(Match::Phase phase) const
{
    return !m_visibleIn || *m_visibleIn == phase;
}

} // namespace engine::ecs
