#include "UiFocus.hpp"

#include "Button.hpp"
#include "Hidden.hpp"
#include "Hovered.hpp"
#include "SoundCue.hpp"
#include "TextField.hpp"
#include "Transform.hpp"

namespace engine::ecs {

bool UiFocus::isVisible(const Registry &registry, const Entity &entity)
{
    return registry.isAlive(entity) && !registry.has<Hidden>(entity);
}

std::optional<Entity> UiFocus::elementAt(const Registry &registry, Vector2 point)
{
    std::optional<Entity> found;
    float smallest = 0.f;
    for (std::size_t i : registry.view<Transform>()) {
        const Entity entity = registry.entityAt(i);
        if (!isVisible(registry, entity))
            continue;
        const Transform &transform = registry.get<Transform>(entity);
        const Vector2 origin(transform.x(), transform.y());
        float area = -1.f;
        if (registry.has<Button>(entity) && registry.get<Button>(entity).contains(origin, point))
            area = registry.get<Button>(entity).width() * registry.get<Button>(entity).height();
        else if (registry.has<TextField>(entity) && registry.get<TextField>(entity).contains(origin, point))
            area = registry.get<TextField>(entity).width() * registry.get<TextField>(entity).height();
        if (area >= 0.f && (!found || area <= smallest)) {
            found = entity;
            smallest = area;
        }
    }
    return found;
}

std::optional<Entity> UiFocus::hovered(const Registry &registry)
{
    const auto entities = registry.view<Hovered>();
    if (entities.empty())
        return std::nullopt;
    return registry.entityAt(entities.front());
}

void UiFocus::hover(Registry &registry, const Entity &entity)
{
    const auto current = hovered(registry);
    if (current && *current == entity)
        return;
    clearHover(registry);
    registry.emplace<Hovered>(entity);
}

void UiFocus::clearHover(Registry &registry)
{
    for (std::size_t i : registry.view<Hovered>())
        registry.remove<Hovered>(registry.entityAt(i));
}

void UiFocus::activateField(Registry &registry, const Entity &field)
{
    auto &fields = registry.getComponents<TextField>();
    for (std::size_t i : registry.view<TextField>())
        fields.at(i).setActive(registry.entityAt(i) == field);
}

void UiFocus::deactivateFields(Registry &registry)
{
    auto &fields = registry.getComponents<TextField>();
    for (std::size_t i : registry.view<TextField>())
        fields.at(i).setActive(false);
}

void UiFocus::press(Registry &registry, const Entity &button, EventQueue<UiActionEvent> &actions)
{
    const Button &target = registry.get<Button>(button);
    actions.push(UiActionEvent(target.action()));
    if (!target.clickSound().empty()) {
        const std::string sound = target.clickSound();
        registry.emplace<SoundCue>(button, sound);
    }
}

} // namespace engine::ecs
