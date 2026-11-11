#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"
#include "UiInput.hpp"

namespace engine::ecs {

class ButtonStyleSystem : public ISystem
{
public:
    ButtonStyleSystem(Registry &registry, const UiInput &input);

    void update(float dt) override;

private:
    Registry &m_registry;
    const UiInput &m_input;
};

} // namespace engine::ecs
