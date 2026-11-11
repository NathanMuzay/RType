#include "PointerSystem.hpp"

#include "Button.hpp"
#include "TextField.hpp"
#include "UiFocus.hpp"

namespace engine::ecs {

PointerSystem::PointerSystem(Registry &registry, const UiInput &input, EventQueue<UiActionEvent> &actions)
    : m_registry(registry), m_input(input), m_actions(actions)
{
}

void PointerSystem::update(float)
{
    if (m_input.pointerMoved()) {
        if (const auto target = UiFocus::elementAt(m_registry, m_input.pointer()))
            UiFocus::hover(m_registry, *target);
        else
            UiFocus::clearHover(m_registry);
    }
    if (!m_input.clicked())
        return;
    const auto target = UiFocus::elementAt(m_registry, m_input.pointer());
    if (!target) {
        UiFocus::deactivateFields(m_registry);
        return;
    }
    UiFocus::hover(m_registry, *target);
    if (m_registry.has<TextField>(*target)) {
        UiFocus::activateField(m_registry, *target);
        return;
    }
    UiFocus::deactivateFields(m_registry);
    if (m_registry.has<Button>(*target))
        UiFocus::press(m_registry, *target, m_actions);
}

} // namespace engine::ecs
