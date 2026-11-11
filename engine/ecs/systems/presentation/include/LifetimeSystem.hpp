#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class LifetimeSystem : public ISystem
{
public:
    static constexpr float MAX_ALPHA = 255.f;

    explicit LifetimeSystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

} // namespace engine::ecs
