#include "ProgressBarSystem.hpp"

#include <algorithm>
#include <cstdlib>

#include "ProgressBar.hpp"
#include "RectShape.hpp"

namespace engine::ecs {

ProgressBarSystem::ProgressBarSystem(Registry &registry, const UiStore &store) : m_registry(registry), m_store(store)
{
}

void ProgressBarSystem::update(float)
{
    const auto &bars = m_registry.getComponents<ProgressBar>();
    auto &shapes = m_registry.getComponents<RectShape>();
    for (std::size_t i : m_registry.view<ProgressBar, RectShape>()) {
        const ProgressBar &bar = bars.at(i);
        if (bar.vertical())
            shapes.at(i).setHeight(bar.width() * ratioOf(bar));
        else
            shapes.at(i).setWidth(bar.width() * ratioOf(bar));
    }
}

float ProgressBarSystem::ratioOf(const ProgressBar &bar) const
{
    const std::string text = m_store.value(bar.key());
    if (text.empty() || bar.maximum() <= 0.f)
        return 0.f;
    return std::clamp(std::strtof(text.c_str(), nullptr) / bar.maximum(), 0.f, 1.f);
}

} // namespace engine::ecs
