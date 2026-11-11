#include "FocusNavigationSystem.hpp"

#include <algorithm>
#include <utility>

#include "Button.hpp"
#include "Focusable.hpp"
#include "TextField.hpp"
#include "UiFocus.hpp"

namespace engine::ecs {

FocusNavigationSystem::FocusNavigationSystem(
    Registry &registry, const UiInput &input, EventQueue<UiActionEvent> &actions)
    : m_registry(registry), m_input(input), m_actions(actions)
{
}

void FocusNavigationSystem::update(float)
{
    if (m_input.keyPressed(UiKey::Down) || m_input.keyPressed(UiKey::Tab))
        moveFocus(1);
    else if (m_input.keyPressed(UiKey::Up))
        moveFocus(-1);
    if (!m_input.keyPressed(UiKey::Enter))
        return;
    const auto current = UiFocus::hovered(m_registry);
    if (current && m_registry.has<Button>(*current))
        UiFocus::press(m_registry, *current, m_actions);
    else if (current && m_registry.has<TextField>(*current)) {
        const std::string &submit = m_registry.get<TextField>(*current).submit();
        if (submit.empty())
            moveFocus(1);
        else
            m_actions.push(UiActionEvent(submit));
    }
}

std::vector<Entity> FocusNavigationSystem::focusOrder() const
{
    std::vector<std::pair<int, std::size_t>> keys;
    for (std::size_t i : m_registry.view<Focusable>()) {
        const Entity entity = m_registry.entityAt(i);
        const bool interactive = m_registry.has<Button>(entity) || m_registry.has<TextField>(entity);
        if (interactive && UiFocus::isVisible(m_registry, entity))
            keys.emplace_back(m_registry.get<Focusable>(entity).order(), i);
    }
    std::sort(keys.begin(), keys.end());
    std::vector<Entity> entities;
    for (const auto &key : keys)
        entities.push_back(m_registry.entityAt(key.second));
    return entities;
}

void FocusNavigationSystem::moveFocus(int step)
{
    const std::vector<Entity> order = focusOrder();
    if (order.empty())
        return;
    const auto current = UiFocus::hovered(m_registry);
    const auto found = current ? std::find(order.begin(), order.end(), *current) : order.end();
    const auto count = static_cast<long>(order.size());
    long index = found == order.end() ? (step > 0 ? 0 : count - 1) : (found - order.begin()) + step;
    index = ((index % count) + count) % count;
    focus(order[static_cast<std::size_t>(index)]);
}

void FocusNavigationSystem::focus(const Entity &entity)
{
    UiFocus::hover(m_registry, entity);
    if (m_registry.has<TextField>(entity))
        UiFocus::activateField(m_registry, entity);
    else
        UiFocus::deactivateFields(m_registry);
}

} // namespace engine::ecs
