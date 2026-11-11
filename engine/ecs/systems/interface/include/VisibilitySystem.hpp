#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"
#include "UiStore.hpp"

namespace engine::ecs {

class VisibilitySystem : public ISystem
{
public:
    VisibilitySystem(Registry &registry, const UiStore &store);

    void update(float dt) override;

private:
    Registry &m_registry;
    const UiStore &m_store;
};

} // namespace engine::ecs
