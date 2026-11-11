#pragma once

#include "Entity.hpp"
#include "IComponent.hpp"

namespace engine::ecs {

class Registry;

class Destroyed : public IComponent
{
public:
    static bool isActive(const Registry &registry, const Entity &entity);
};

} // namespace engine::ecs
