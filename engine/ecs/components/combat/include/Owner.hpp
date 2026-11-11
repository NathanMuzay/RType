#pragma once

#include "Entity.hpp"
#include "IComponent.hpp"

namespace engine::ecs {

class Owner : public IComponent
{
public:
    explicit Owner(Entity owner);

    Entity owner() const;

private:
    Entity m_owner;
};

} // namespace engine::ecs
