#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class CleanupSystem : public ISystem
{
public:
    explicit CleanupSystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

} // namespace engine::ecs
