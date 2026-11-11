#include "Destroyed.hpp"

#include "Registry.hpp"

namespace engine::ecs {

bool Destroyed::isActive(const Registry &registry, const Entity &entity)
{
    return registry.isAlive(entity) && !registry.has<Destroyed>(entity);
}

} // namespace engine::ecs
