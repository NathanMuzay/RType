#pragma once

#include <random>
#include <string>

#include "IRenderer.hpp"
#include "ISystem.hpp"
#include "UiStore.hpp"

namespace engine::ecs {

class ShakeSystem : public ISystem
{
public:
    ShakeSystem(const UiStore &store, render::IRenderer &renderer, std::string key, float strength);

    void update(float dt) override;

private:
    const UiStore &m_store;
    render::IRenderer &m_renderer;
    std::string m_key;
    float m_strength;
    std::mt19937 m_random;
};

} // namespace engine::ecs
