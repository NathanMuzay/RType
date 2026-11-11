#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class OffscreenSystem : public ISystem
{
public:
    explicit OffscreenSystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

} // namespace engine::ecs
