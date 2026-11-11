#include "TiltAnimationSystem.hpp"

#include "PlayerIndex.hpp"
#include "Sprite.hpp"
#include "TiltAnimation.hpp"

namespace engine::ecs {

TiltAnimationSystem::TiltAnimationSystem(Registry &registry) : m_registry(registry)
{
}

void TiltAnimationSystem::update(float dt)
{
    auto &animations = m_registry.getComponents<TiltAnimation>();
    auto &sprites = m_registry.getComponents<Sprite>();
    const auto &indices = m_registry.getComponents<PlayerIndex>();
    for (std::size_t i : m_registry.view<TiltAnimation, Sprite>()) {
        if (m_registry.has<PlayerIndex>(m_registry.entityAt(i)))
            animations.at(i).setRow(static_cast<int>(indices.at(i).slot()));
        animations.at(i).advance(dt);
        sprites.at(i).setSource(animations.at(i).currentFrame());
    }
}

} // namespace engine::ecs
