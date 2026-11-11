#pragma once

#include <cstddef>
#include <optional>
#include <string>

#include "IComponent.hpp"
#include "Match.hpp"

namespace engine::ecs {

class StatusText : public IComponent
{
public:
    StatusText(std::string format, std::optional<std::size_t> slot, std::optional<Match::Phase> visibleIn);

    const std::string &format() const;
    std::optional<std::size_t> slot() const;
    bool visibleIn(Match::Phase phase) const;

private:
    std::string m_format;
    std::optional<std::size_t> m_slot;
    std::optional<Match::Phase> m_visibleIn;
};

} // namespace engine::ecs
