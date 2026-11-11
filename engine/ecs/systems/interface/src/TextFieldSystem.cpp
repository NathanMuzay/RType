#include "TextFieldSystem.hpp"

#include "Text.hpp"
#include "TextField.hpp"

namespace engine::ecs {

TextFieldSystem::TextFieldSystem(Registry &registry, const UiInput &input, UiStore &store)
    : m_registry(registry), m_input(input), m_store(store)
{
}

void TextFieldSystem::update(float)
{
    auto &fields = m_registry.getComponents<TextField>();
    for (std::size_t i : m_registry.view<TextField>()) {
        TextField &field = fields.at(i);
        if (field.active()) {
            field.append(m_input.typedText());
            if (m_input.keyPressed(UiKey::Backspace))
                field.erase();
        }
        const Entity entity = m_registry.entityAt(i);
        if (m_registry.has<Text>(entity))
            m_registry.get<Text>(entity).setContent(field.display());
        m_store.setValue(std::string(STORE_PREFIX) + field.name(), field.content());
    }
}

} // namespace engine::ecs
