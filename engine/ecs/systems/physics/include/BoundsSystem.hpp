#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class BoundsSystem : public ISystem
{
public:
    explicit BoundsSystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

} // namespace engine::ecs
