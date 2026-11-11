#pragma once

#include <random>

#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class ParticleSystem : public ISystem
{
public:
    explicit ParticleSystem(Registry &registry);

    void update(float dt) override;

private:
    void emit(const Entity &source);

    Registry &m_registry;
    std::mt19937 m_random;
};

} // namespace engine::ecs
