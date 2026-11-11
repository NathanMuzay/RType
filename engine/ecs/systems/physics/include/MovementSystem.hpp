#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class MovementSystem : public ISystem
{
public:
    explicit MovementSystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

} // namespace engine::ecs
