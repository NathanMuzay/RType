#include "StatusTextSystem.hpp"

#include <cstdint>

#include "Lives.hpp"
#include "MatchSystem.hpp"
#include "Player.hpp"
#include "Score.hpp"
#include "Text.hpp"
#include "TextFormat.hpp"

namespace engine::ecs {

StatusTextSystem::StatusTextSystem(Registry &registry) : m_registry(registry)
{
}

void StatusTextSystem::update(float)
{
    const Match::Phase phase = MatchSystem::phaseOf(m_registry);
    const auto &statuses = m_registry.getComponents<StatusText>();
    auto &texts = m_registry.getComponents<Text>();
    for (std::size_t i : m_registry.view<StatusText, Text>()) {
        const StatusText &status = statuses.at(i);
        texts.at(i).setContent(status.visibleIn(phase) ? render(status) : std::string());
    }
}

std::string StatusTextSystem::render(const StatusText &status) const
{
    const auto slot = status.slot();
    if (!slot)
        return status.format();
    const auto &players = m_registry.getComponents<Player>();
    for (std::size_t i : m_registry.view<Player>()) {
        if (players.at(i).slot() != *slot)
            continue;
        const Entity owner = m_registry.entityAt(i);
        const int lives = m_registry.has<Lives>(owner) ? m_registry.get<Lives>(owner).count() : 0;
        const std::uint32_t score = m_registry.has<Score>(owner) ? m_registry.get<Score>(owner).points() : 0;
        std::string text = replaceToken(status.format(), "{slot}", std::to_string(*slot + 1));
        text = replaceToken(text, "{lives}", std::to_string(lives));
        return replaceToken(text, "{score}", std::to_string(score));
    }
    return {};
}

} // namespace engine::ecs
