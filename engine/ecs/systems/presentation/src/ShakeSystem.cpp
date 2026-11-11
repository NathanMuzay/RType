#include "ShakeSystem.hpp"

#include <cstdlib>
#include <utility>

namespace engine::ecs {

ShakeSystem::ShakeSystem(const UiStore &store, render::IRenderer &renderer, std::string key, float strength)
    : m_store(store), m_renderer(renderer), m_key(std::move(key)), m_strength(strength),
      m_random(std::random_device{}())
{
}

void ShakeSystem::update(float)
{
    const std::string text = m_store.value(m_key);
    const float amount = text.empty() ? 0.f : std::strtof(text.c_str(), nullptr) * m_strength;
    if (amount <= 0.f) {
        m_renderer.setOffset({0.f, 0.f});
        return;
    }
    std::uniform_real_distribution<float> offset(-amount, amount);
    m_renderer.setOffset({offset(m_random), offset(m_random)});
}

} // namespace engine::ecs
