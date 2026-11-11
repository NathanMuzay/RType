#pragma once

#include <optional>

#include "Entity.hpp"
#include "EventQueue.hpp"
#include "Registry.hpp"
#include "UiActionEvent.hpp"
#include "Vector2.hpp"

namespace engine::ecs {

class UiFocus
{
public:
    static bool isVisible(const Registry &registry, const Entity &entity);
    static std::optional<Entity> elementAt(const Registry &registry, Vector2 point);
    static std::optional<Entity> hovered(const Registry &registry);
    static void hover(Registry &registry, const Entity &entity);
    static void clearHover(Registry &registry);
    static void activateField(Registry &registry, const Entity &field);
    static void deactivateFields(Registry &registry);
    static void press(Registry &registry, const Entity &button, EventQueue<UiActionEvent> &actions);
};

} // namespace engine::ecs
