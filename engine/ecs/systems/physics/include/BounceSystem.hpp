#pragma once

#include <cstddef>

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class BounceSystem : public ISystem
{
public:
    explicit BounceSystem(Registry &registry);

    void update(float dt) override;

private:
    void bounceOnBounds(std::size_t index);
    void bounceOnSolid(std::size_t index, std::size_t wall);

    Registry &m_registry;
};

} // namespace engine::ecs
