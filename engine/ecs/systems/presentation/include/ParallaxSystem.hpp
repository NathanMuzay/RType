#pragma once

#include <cstddef>
#include <string>

#include "ISystem.hpp"
#include "Registry.hpp"
#include "UiStore.hpp"

namespace engine::ecs {

class ParallaxSystem : public ISystem
{
public:
    static constexpr float BOOST_EASING = 1.5f;

    explicit ParallaxSystem(Registry &registry);
    ParallaxSystem(Registry &registry, const UiStore &store, std::string boostKey, std::string boostValue, float boost);

    void update(float dt) override;

private:
    void updateSpeed(float dt);
    void scroll(std::size_t index, float width, float dt);

    Registry &m_registry;
    const UiStore *m_store;
    std::string m_boostKey;
    std::string m_boostValue;
    float m_boost;
    float m_speedFactor;
};

} // namespace engine::ecs
