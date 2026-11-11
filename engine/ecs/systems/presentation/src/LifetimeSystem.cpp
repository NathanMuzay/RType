#include "LifetimeSystem.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include "Lifetime.hpp"
#include "RectShape.hpp"

namespace engine::ecs {

LifetimeSystem::LifetimeSystem(Registry &registry) : m_registry(registry)
{
}

void LifetimeSystem::update(float dt)
{
    std::vector<Entity> expired;
    for (std::size_t i : m_registry.view<Lifetime>()) {
        const Entity entity = m_registry.entityAt(i);
        Lifetime &life = m_registry.get<Lifetime>(entity);
        life.elapse(dt);
        if (life.remaining() <= 0.f) {
            expired.push_back(entity);
            continue;
        }
        if (m_registry.has<RectShape>(entity) && life.total() > 0.f) {
            RectShape &shape = m_registry.get<RectShape>(entity);
            const Color color = shape.color();
            const float ratio = life.remaining() / life.total();
            shape.setColor(Color(color.r(), color.g(), color.b(), static_cast<std::uint8_t>(ratio * MAX_ALPHA)));
        }
    }
    for (const Entity &entity : expired)
        m_registry.kill(entity);
}

} // namespace engine::ecs
