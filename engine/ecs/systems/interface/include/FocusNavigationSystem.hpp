#pragma once

#include <vector>

#include "Entity.hpp"
#include "EventQueue.hpp"
#include "ISystem.hpp"
#include "Registry.hpp"
#include "UiActionEvent.hpp"
#include "UiInput.hpp"

namespace engine::ecs {

class FocusNavigationSystem : public ISystem
{
public:
    FocusNavigationSystem(Registry &registry, const UiInput &input, EventQueue<UiActionEvent> &actions);

    void update(float dt) override;

private:
    std::vector<Entity> focusOrder() const;
    void moveFocus(int step);
    void focus(const Entity &entity);

    Registry &m_registry;
    const UiInput &m_input;
    EventQueue<UiActionEvent> &m_actions;
};

} // namespace engine::ecs
