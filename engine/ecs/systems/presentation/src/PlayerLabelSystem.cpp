#include "PlayerLabelSystem.hpp"

#include <string>

#include "Anchor.hpp"
#include "Color.hpp"
#include "PlayerIndex.hpp"
#include "PlayerLabel.hpp"
#include "Text.hpp"
#include "TextFormat.hpp"
#include "Transform.hpp"

namespace engine::ecs {

PlayerLabelSystem::PlayerLabelSystem(Registry &registry, bool enabled) : m_registry(registry), m_enabled(enabled)
{
}

void PlayerLabelSystem::update(float)
{
    if (!m_enabled)
        return;
    for (std::size_t i : m_registry.view<PlayerLabel, PlayerIndex, Transform>()) {
        const Entity owner = m_registry.entityAt(i);
        PlayerLabel &label = m_registry.get<PlayerLabel>(owner);
        if (label.attached())
            continue;
        label.markAttached();
        const std::size_t slot = m_registry.get<PlayerIndex>(owner).slot();
        const std::string font = label.font();
        const std::string content = replaceToken(label.format(), "{slot}", std::to_string(slot + 1));
        const unsigned int size = label.size();
        const Color color = label.colorFor(slot);
        const float offsetX = label.offsetX();
        const float offsetY = label.offsetY();
        const Transform &position = m_registry.get<Transform>(owner);
        const float x = position.x() + offsetX;
        const float y = position.y() + offsetY;

        const Entity text = m_registry.spawn();
        m_registry.emplace<Transform>(text, x, y);
        m_registry.emplace<Text>(text, font, content, size, color);
        m_registry.emplace<Anchor>(text, owner, offsetX, offsetY);
    }
}

} // namespace engine::ecs
