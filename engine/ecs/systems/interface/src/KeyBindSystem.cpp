#include "KeyBindSystem.hpp"

#include "Hovered.hpp"
#include "UiFocus.hpp"

namespace engine::ecs {

KeyBindSystem::KeyBindSystem(Registry &registry, const UiInput &input, UiStore &store)
    : m_registry(registry), m_input(input), m_store(store), m_releasing(false)
{
}

void KeyBindSystem::update(float)
{
    if (m_releasing) {
        m_store.setValue(CAPTURING_KEY, NO);
        m_releasing = false;
        return;
    }
    for (std::size_t i : m_registry.view<KeyBind>()) {
        const Entity entity = m_registry.entityAt(i);
        KeyBind &bind = m_registry.get<KeyBind>(entity);
        if (bind.waiting()) {
            capture(entity, bind);
            return;
        }
    }
    if (!activated())
        return;
    for (std::size_t i : m_registry.view<KeyBind, Hovered>()) {
        KeyBind &bind = m_registry.getComponents<KeyBind>().at(i);
        bind.startWaiting(m_store.value(bind.storeKey()));
        m_store.setValue(bind.storeKey(), bind.prompt());
        m_store.setValue(CAPTURING_KEY, YES);
        return;
    }
}

void KeyBindSystem::capture(const Entity &entity, KeyBind &bind)
{
    UiFocus::hover(m_registry, entity);
    if (!bind.armed()) {
        bind.arm();
        return;
    }
    if (m_input.keyPressed(UiKey::Escape)) {
        finish(bind, bind.previous());
        return;
    }
    if (const auto value = captured(bind))
        finish(bind, *value);
}

void KeyBindSystem::finish(KeyBind &bind, const std::string &value)
{
    m_store.setValue(bind.storeKey(), value);
    m_releasing = true;
    bind.stopWaiting();
}

std::optional<std::string> KeyBindSystem::captured(const KeyBind &bind) const
{
    if (bind.gamepad()) {
        if (const auto button = m_input.rawButton())
            return RAW_PREFIX + std::to_string(*button);
        return std::nullopt;
    }
    if (const auto key = m_input.rawKey())
        return RAW_PREFIX + std::to_string(*key);
    return std::nullopt;
}

bool KeyBindSystem::activated() const
{
    return m_input.keyPressed(UiKey::Enter) || m_input.clicked();
}

} // namespace engine::ecs
