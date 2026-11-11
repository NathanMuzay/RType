#include "CleanupSystem.hpp"

#include "Destroyed.hpp"

namespace engine::ecs {

CleanupSystem::CleanupSystem(Registry &registry) : m_registry(registry)
{
}

void CleanupSystem::update(float)
{
    for (std::size_t i : m_registry.view<Destroyed>())
        m_registry.kill(m_registry.entityAt(i));
}

} // namespace engine::ecs
