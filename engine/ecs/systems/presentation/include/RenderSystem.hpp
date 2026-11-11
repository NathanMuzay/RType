#pragma once

#include <cstdint>

#include <cstddef>

#include "Color.hpp"
#include "IRenderer.hpp"
#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class RenderSystem : public ISystem
{
public:
    static constexpr std::uint8_t FLASH_RED = 255;
    static constexpr std::uint8_t FLASH_OTHER = 90;
    static constexpr float MAX_ALPHA = 255.f;

    RenderSystem(Registry &registry, render::IRenderer &renderer, Color background);

    void update(float dt) override;

private:
    enum class Layer
    {
        Background,
        World,
        Foreground
    };

    void drawLayer(Layer layer);
    void drawTexts(bool foreground);
    bool isHidden(std::size_t index) const;
    Layer layerOf(std::size_t index) const;
    float rotationOf(std::size_t index) const;
    Color tintOf(std::size_t index) const;

    Registry &m_registry;
    render::IRenderer &m_renderer;
    Color m_background;
};

} // namespace engine::ecs
