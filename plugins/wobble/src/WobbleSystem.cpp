#include "WobbleSystem.hpp"

#include <cmath>
#include <utility>

#include "Rotation.hpp"
#include "Sprite.hpp"

namespace plugins::wobble {

WobbleSystem::WobbleSystem(engine::ecs::Registry &registry, std::string texture, float amplitude, float speed)
    : m_registry(registry), m_texture(std::move(texture)), m_amplitude(amplitude), m_speed(speed), m_time(0.f)
{
}

void WobbleSystem::update(float dt)
{
    m_time += dt;
    const float angle = m_amplitude * std::sin(m_time * m_speed);
    for (std::size_t i : m_registry.view<engine::ecs::Sprite>()) {
        const engine::ecs::Entity entity = m_registry.entityAt(i);
        if (m_texture.empty() || m_registry.get<engine::ecs::Sprite>(entity).texture() == m_texture)
            m_registry.emplace<engine::ecs::Rotation>(entity, angle);
    }
}

std::unique_ptr<engine::ecs::ISystem> WobbleSystem::build(
    engine::scene::SystemContext &context, const engine::scene::DataNode &entry)
{
    if (entry.isString())
        return std::make_unique<WobbleSystem>(context.registry(), std::string(), DEFAULT_AMPLITUDE, DEFAULT_SPEED);
    const std::string texture = entry.has("texture") ? entry.getString("texture") : std::string();
    return std::make_unique<WobbleSystem>(context.registry(), texture, entry.getFloat("amplitude", DEFAULT_AMPLITUDE),
        entry.getFloat("speed", DEFAULT_SPEED));
}

} // namespace plugins::wobble
