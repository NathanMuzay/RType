#pragma once

#include "EventQueue.hpp"
#include "ISystem.hpp"
#include "Registry.hpp"
#include "UiActionEvent.hpp"
#include "UiInput.hpp"

namespace engine::ecs {

class PointerSystem : public ISystem
{
public:
    PointerSystem(Registry &registry, const UiInput &input, EventQueue<UiActionEvent> &actions);

    void update(float dt) override;

private:
    Registry &m_registry;
    const UiInput &m_input;
    EventQueue<UiActionEvent> &m_actions;
};

} // namespace engine::ecs
