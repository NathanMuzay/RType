#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"
#include "UiInput.hpp"
#include "UiStore.hpp"

namespace engine::ecs {

class SettingSliderSystem : public ISystem
{
public:
    SettingSliderSystem(Registry &registry, const UiInput &input, UiStore &store);

    void update(float dt) override;

private:
    int direction() const;

    Registry &m_registry;
    const UiInput &m_input;
    UiStore &m_store;
};

} // namespace engine::ecs
