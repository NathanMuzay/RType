#include "SettingSliderSystem.hpp"

#include "Hovered.hpp"
#include "KeyBindSystem.hpp"
#include "SettingSlider.hpp"

namespace engine::ecs {

SettingSliderSystem::SettingSliderSystem(Registry &registry, const UiInput &input, UiStore &store)
    : m_registry(registry), m_input(input), m_store(store)
{
}

void SettingSliderSystem::update(float)
{
    if (m_store.value(KeyBindSystem::CAPTURING_KEY) == KeyBindSystem::YES)
        return;
    const int keys = direction();
    for (std::size_t i : m_registry.view<SettingSlider, Hovered>()) {
        const SettingSlider &slider = m_registry.getComponents<SettingSlider>().at(i);
        const int step = slider.direction() == 0 ? keys : (m_input.clicked() ? slider.direction() : 0);
        if (step != 0)
            m_store.setValue(slider.storeKey(), slider.step(m_store.value(slider.storeKey()), step));
    }
}

int SettingSliderSystem::direction() const
{
    if (m_input.keyPressed(UiKey::Right) || m_input.keyPressed(UiKey::Enter))
        return 1;
    if (m_input.keyPressed(UiKey::Left))
        return -1;
    return 0;
}

} // namespace engine::ecs
