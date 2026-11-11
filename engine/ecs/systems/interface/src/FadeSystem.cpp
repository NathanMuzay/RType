#include "FadeSystem.hpp"

#include <cstdint>

#include "FadeWhen.hpp"
#include "RectShape.hpp"

namespace engine::ecs {

FadeSystem::FadeSystem(Registry &registry, const UiStore &store) : m_registry(registry), m_store(store)
{
}

void FadeSystem::update(float dt)
{
    auto &fades = m_registry.getComponents<FadeWhen>();
    for (std::size_t i : m_registry.view<FadeWhen>()) {
        FadeWhen &fade = fades.at(i);
        fade.fade(m_store.value(fade.key()) == fade.value(), dt);
    }
    auto &shapes = m_registry.getComponents<RectShape>();
    for (std::size_t i : m_registry.view<FadeWhen, RectShape>()) {
        const Color color = shapes.at(i).color();
        const auto alpha = static_cast<std::uint8_t>(fades.at(i).opacity() * MAX_ALPHA);
        shapes.at(i).setColor(Color(color.r(), color.g(), color.b(), alpha));
    }
}

} // namespace engine::ecs
