#include "HitFlashSystem.hpp"

#include <cstddef>

#include "HitFlash.hpp"

namespace engine::ecs {

HitFlashSystem::HitFlashSystem(Registry &registry) : m_registry(registry)
{
}

void HitFlashSystem::update(float dt)
{
    for (std::size_t i : m_registry.view<HitFlash>()) {
        const Entity entity = m_registry.entityAt(i);
        HitFlash &flash = m_registry.get<HitFlash>(entity);
        flash.elapse(dt);
        if (flash.remaining() <= 0.f)
            m_registry.remove<HitFlash>(entity);
    }
}

} // namespace engine::ecs
