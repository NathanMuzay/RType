#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class PlayerControlSystem : public ISystem
{
public:
    explicit PlayerControlSystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

} // namespace engine::ecs
