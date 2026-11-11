#include "ButtonStyleSystem.hpp"

#include <string>

#include "Button.hpp"
#include "Hovered.hpp"
#include "RectShape.hpp"
#include "SoundCue.hpp"
#include "Sprite.hpp"

namespace engine::ecs {

ButtonStyleSystem::ButtonStyleSystem(Registry &registry, const UiInput &input) : m_registry(registry), m_input(input)
{
}

void ButtonStyleSystem::update(float)
{
    for (std::size_t i : m_registry.view<Button>()) {
        const Entity entity = m_registry.entityAt(i);
        Button &button = m_registry.get<Button>(entity);
        const bool hovered = m_registry.has<Hovered>(entity);
        Button::State state = Button::State::Normal;
        if (hovered)
            state = m_input.pointerDown() ? Button::State::Pressed : Button::State::Hovered;
        if (const auto color = button.colorFor(state); color && m_registry.has<RectShape>(entity))
            m_registry.get<RectShape>(entity).setColor(*color);
        if (const auto frame = button.frameFor(state); frame && m_registry.has<Sprite>(entity))
            m_registry.get<Sprite>(entity).setSource(*frame);
        const bool enteredHover = hovered && !button.wasHovered();
        button.setWasHovered(hovered);
        if (enteredHover && !button.hoverSound().empty()) {
            const std::string sound = button.hoverSound();
            m_registry.emplace<SoundCue>(entity, sound);
        }
    }
}

} // namespace engine::ecs
