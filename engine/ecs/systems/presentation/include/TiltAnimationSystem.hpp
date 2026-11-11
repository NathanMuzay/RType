#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class TiltAnimationSystem : public ISystem
{
public:
    explicit TiltAnimationSystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

} // namespace engine::ecs
