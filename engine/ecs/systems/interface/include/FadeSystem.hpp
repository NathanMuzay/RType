#pragma once

#include "ISystem.hpp"
#include "Registry.hpp"
#include "UiStore.hpp"

namespace engine::ecs {

class FadeSystem : public ISystem
{
public:
    static constexpr float MAX_ALPHA = 255.f;

    FadeSystem(Registry &registry, const UiStore &store);

    void update(float dt) override;

private:
    Registry &m_registry;
    const UiStore &m_store;
};

} // namespace engine::ecs
