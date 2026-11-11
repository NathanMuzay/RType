#include "SpriteAnimationSystem.hpp"

#include "Destroyed.hpp"
#include "Sprite.hpp"
#include "SpriteAnimation.hpp"

namespace engine::ecs {

SpriteAnimationSystem::SpriteAnimationSystem(Registry &registry) : m_registry(registry)
{
}

void SpriteAnimationSystem::update(float dt)
{
    auto &animations = m_registry.getComponents<SpriteAnimation>();
    auto &sprites = m_registry.getComponents<Sprite>();
    for (std::size_t i : m_registry.view<SpriteAnimation, Sprite>()) {
        SpriteAnimation &animation = animations.at(i);
        animation.advance(dt);
        sprites.at(i).setSource(animation.currentFrame());
        const Entity entity = m_registry.entityAt(i);
        if (animation.finished() && !m_registry.has<Destroyed>(entity))
            m_registry.emplace<Destroyed>(entity);
    }
}

} // namespace engine::ecs
