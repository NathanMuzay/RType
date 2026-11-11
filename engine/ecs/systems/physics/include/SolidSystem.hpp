#pragma once

#include <cstddef>

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class SolidSystem : public ISystem
{
public:
    explicit SolidSystem(Registry &registry);

    void update(float dt) override;

private:
    void pushOut(std::size_t mover, std::size_t wall);

    Registry &m_registry;
};

} // namespace engine::ecs
