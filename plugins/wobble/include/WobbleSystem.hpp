#pragma once

#include <memory>
#include <string>

#include "DataNode.hpp"
#include "ISystem.hpp"
#include "Registry.hpp"
#include "SystemContext.hpp"

namespace plugins::wobble {

class WobbleSystem : public engine::ecs::ISystem
{
public:
    static constexpr float DEFAULT_AMPLITUDE = 12.f;
    static constexpr float DEFAULT_SPEED = 3.f;

    WobbleSystem(engine::ecs::Registry &registry, std::string texture, float amplitude, float speed);

    void update(float dt) override;

    static std::unique_ptr<engine::ecs::ISystem> build(
        engine::scene::SystemContext &context, const engine::scene::DataNode &entry);

private:
    engine::ecs::Registry &m_registry;
    std::string m_texture;
    float m_amplitude;
    float m_speed;
    float m_time;
};

} // namespace plugins::wobble
