#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class AnchorSystem : public ISystem
{
public:
    explicit AnchorSystem(Registry &registry);

    void update(float dt) override;

private:
    Registry &m_registry;
};

} // namespace engine::ecs
