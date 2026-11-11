#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class PlayerLabelSystem : public ISystem
{
public:
    PlayerLabelSystem(Registry &registry, bool enabled);

    void update(float dt) override;

private:
    Registry &m_registry;
    bool m_enabled;
};

} // namespace engine::ecs
