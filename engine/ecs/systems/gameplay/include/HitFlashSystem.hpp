#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class HitFlashSystem : public ISystem
{
public:
    explicit HitFlashSystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

} // namespace engine::ecs
