#include "RenderSystem.hpp"

#include "Background.hpp"
#include "FadeWhen.hpp"
#include "Foreground.hpp"
#include "Hidden.hpp"
#include "HitFlash.hpp"
#include "RectShape.hpp"
#include "Rotation.hpp"
#include "Sprite.hpp"
#include "Text.hpp"
#include "Transform.hpp"

namespace engine::ecs {

RenderSystem::RenderSystem(Registry &registry, render::IRenderer &renderer, Color background)
    : m_registry(registry), m_renderer(renderer), m_background(background)
{
}

void RenderSystem::update(float)
{
    m_renderer.clear(m_background);
    const Vector2 shake = m_renderer.offset();
    drawLayer(Layer::Background);
    drawLayer(Layer::World);
    m_renderer.setOffset({0.f, 0.f});
    drawTexts(false);
    drawLayer(Layer::Foreground);
    drawTexts(true);
    m_renderer.setOffset(shake);
    m_renderer.display();
}

void RenderSystem::drawLayer(Layer layer)
{
    const auto &transforms = m_registry.getComponents<Transform>();
    const auto &shapes = m_registry.getComponents<RectShape>();
    const auto &sprites = m_registry.getComponents<Sprite>();
    for (std::size_t i : m_registry.view<Transform, RectShape>()) {
        if (isHidden(i) || layerOf(i) != layer)
            continue;
        const Transform &transform = transforms.at(i);
        const RectShape &shape = shapes.at(i);
        if (shape.color().a() == 0 && shape.thickness() <= 0.f)
            continue;
        m_renderer.drawRect({transform.x(), transform.y()}, {shape.width(), shape.height()}, shape.color(),
            shape.outline(), shape.thickness());
    }
    for (std::size_t i : m_registry.view<Transform, Sprite>()) {
        if (isHidden(i) || layerOf(i) != layer)
            continue;
        const Transform &transform = transforms.at(i);
        const Sprite &sprite = sprites.at(i);
        const Color tint = tintOf(i);
        if (tint.a() == 0)
            continue;
        m_renderer.drawTexture(sprite.texture(), sprite.source(), {transform.x(), transform.y()},
            {sprite.scale(), sprite.scale()}, rotationOf(i), tint);
    }
}

void RenderSystem::drawTexts(bool foreground)
{
    const auto &transforms = m_registry.getComponents<Transform>();
    const auto &texts = m_registry.getComponents<Text>();
    for (std::size_t i : m_registry.view<Transform, Text>()) {
        const Text &text = texts.at(i);
        if (isHidden(i) || text.content().empty() || (layerOf(i) == Layer::Foreground) != foreground)
            continue;
        const Transform &transform = transforms.at(i);
        m_renderer.drawText(text.font(), text.content(), text.size(),
            {transform.x() + text.offsetX(), transform.y() + text.offsetY()}, text.color());
    }
}

bool RenderSystem::isHidden(std::size_t index) const
{
    return m_registry.has<Hidden>(m_registry.entityAt(index));
}

RenderSystem::Layer RenderSystem::layerOf(std::size_t index) const
{
    const Entity entity = m_registry.entityAt(index);
    if (m_registry.has<Foreground>(entity))
        return Layer::Foreground;
    return m_registry.has<Background>(entity) ? Layer::Background : Layer::World;
}

float RenderSystem::rotationOf(std::size_t index) const
{
    const Entity entity = m_registry.entityAt(index);
    return m_registry.has<Rotation>(entity) ? m_registry.get<Rotation>(entity).degrees() : 0.f;
}

Color RenderSystem::tintOf(std::size_t index) const
{
    const Entity entity = m_registry.entityAt(index);
    const Color tint = m_registry.has<HitFlash>(entity) ? Color(FLASH_RED, FLASH_OTHER, FLASH_OTHER) : Color();
    if (!m_registry.has<FadeWhen>(entity))
        return tint;
    const float opacity = m_registry.get<FadeWhen>(entity).opacity();
    return Color(tint.r(), tint.g(), tint.b(), static_cast<std::uint8_t>(opacity * MAX_ALPHA));
}

} // namespace engine::ecs
