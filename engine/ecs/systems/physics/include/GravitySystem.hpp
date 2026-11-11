#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class GravitySystem : public ISystem
{
public:
    explicit GravitySystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

} // namespace engine::ecs
