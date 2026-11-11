#pragma once

#include "ISystem.hpp"
#include "ProgressBar.hpp"
#include "Registry.hpp"
#include "UiStore.hpp"

namespace engine::ecs {

class ProgressBarSystem : public ISystem
{
public:
    ProgressBarSystem(Registry &registry, const UiStore &store);

    void update(float dt) override;

private:
    float ratioOf(const ProgressBar &bar) const;

    Registry &m_registry;
    const UiStore &m_store;
};

} // namespace engine::ecs
