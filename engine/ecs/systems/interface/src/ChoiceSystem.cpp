#include "ChoiceSystem.hpp"

#include <cstddef>
#include <string>

#include "Choice.hpp"
#include "Hovered.hpp"

namespace engine::ecs {

ChoiceSystem::ChoiceSystem(Registry &registry, const UiInput &input, UiStore &store)
    : m_registry(registry), m_input(input), m_store(store)
{
}

void ChoiceSystem::update(float)
{
    for (std::size_t i : m_registry.view<Choice>()) {
        const Entity entity = m_registry.entityAt(i);
        Choice &choice = m_registry.get<Choice>(entity);
        const bool hovered = m_registry.has<Hovered>(entity);
        if (hovered && (m_input.clicked() || m_input.keyPressed(UiKey::Right) || m_input.keyPressed(UiKey::Enter)))
            choice.next();
        else if (hovered && m_input.keyPressed(UiKey::Left))
            choice.previous();
        m_store.setValue(std::string(Choice::STORE_PREFIX) + choice.name(), choice.current());
    }
}

} // namespace engine::ecs
