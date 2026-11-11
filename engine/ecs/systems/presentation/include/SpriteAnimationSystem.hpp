#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class SpriteAnimationSystem : public ISystem
{
public:
    explicit SpriteAnimationSystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

} // namespace engine::ecs
