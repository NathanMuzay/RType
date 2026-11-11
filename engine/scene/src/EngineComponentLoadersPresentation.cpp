#include "EngineComponentLoaders.hpp"

#include <cstdint>
#include <functional>
#include <utility>

#include "Bounce.hpp"
#include "Bounded.hpp"
#include "BoundText.hpp"
#include "Button.hpp"
#include "Choice.hpp"
#include "Collider.hpp"
#include "ContactDamage.hpp"
#include "DataReaders.hpp"
#include "DeathEffect.hpp"
#include "DespawnOffscreen.hpp"
#include "FadeWhen.hpp"
#include "Focusable.hpp"
#include "Foreground.hpp"
#include "Gravity.hpp"
#include "Health.hpp"
#include "Hidden.hpp"
#include "Invulnerable.hpp"
#include "Lifetime.hpp"
#include "ListView.hpp"
#include "Lives.hpp"
#include "MusicWhen.hpp"
#include "Parallax.hpp"
#include "ParticleEmitter.hpp"
#include "Persistent.hpp"
#include "PlayerControl.hpp"
#include "PlayerLabel.hpp"
#include "ProgressBar.hpp"
#include "Pushable.hpp"
#include "RectShape.hpp"
#include "Replicated.hpp"
#include "Respawn.hpp"
#include "Rotation.hpp"
#include "SceneError.hpp"
#include "Score.hpp"
#include "ScoreValue.hpp"
#include "Script.hpp"
#include "Solid.hpp"
#include "SoundCue.hpp"
#include "Sprite.hpp"
#include "SpriteAnimation.hpp"
#include "StatusText.hpp"
#include "Text.hpp"
#include "TextField.hpp"
#include "Tilemap.hpp"
#include "TiltAnimation.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"
#include "VisibleWhen.hpp"
#include "Weapon.hpp"

namespace engine::scene {

void EngineComponentLoaders::loadParticleEmitter(
    ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::ParticleEmitter>(entity, data.getInt("count"), data.getFloat("speed"),
        data.getFloat("lifetime"), data.getFloat("size", 4.f), DataReaders::readColor(data, "color"));
}

void EngineComponentLoaders::loadLifetime(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Lifetime>(entity, data.getFloat("seconds"));
}

void EngineComponentLoaders::loadMusicWhen(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::MusicWhen>(entity, data.getString("key"), data.getString("value"), data.getString("music"));
}

void EngineComponentLoaders::loadSprite(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Sprite>(
        entity, data.getString("texture"), DataReaders::readRect(data, "frame"), data.getFloat("scale", 1.f));
}

void EngineComponentLoaders::loadRectShape(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    if (!data.has("outline")) {
        registry.emplace<ecs::RectShape>(
            entity, data.getFloat("width"), data.getFloat("height"), DataReaders::readColor(data, "color"));
        return;
    }
    registry.emplace<ecs::RectShape>(entity, data.getFloat("width"), data.getFloat("height"),
        DataReaders::readColor(data, "color"), DataReaders::readColor(data, "outline"),
        data.getFloat("thickness", 1.f));
}

void EngineComponentLoaders::loadParallax(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Parallax>(entity, data.getFloat("speed"), data.getFloat("wrapWidth"));
}

void EngineComponentLoaders::loadTiltAnimation(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::TiltAnimation>(entity, DataReaders::readRect(data, "frame"), data.getInt("row", 0),
        data.getInt("flatColumn"), data.getInt("maxTilt"), data.getFloat("framesPerSecond"));
}

void EngineComponentLoaders::loadSpriteAnimation(
    ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::SpriteAnimation>(entity, DataReaders::readRect(data, "frame"), data.getInt("frameCount"),
        data.getInt("stepX"), data.getFloat("framesPerSecond"), data.getBool("loop", false));
}

void EngineComponentLoaders::loadSoundCue(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::SoundCue>(entity, data.getString("sound"));
}

void EngineComponentLoaders::loadText(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    ecs::Text &text = registry.emplace<ecs::Text>(entity, data.getString("font"), readOptionalString(data, "content"),
        static_cast<unsigned int>(data.getInt("size")), DataReaders::readColor(data, "color"));
    text.setOffset(data.getFloat("offsetX", 0.f), data.getFloat("offsetY", 0.f));
}

void EngineComponentLoaders::loadDeathEffect(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::DeathEffect>(entity, data.getString("prefab"));
}

void EngineComponentLoaders::loadPlayerLabel(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::PlayerLabel>(entity, data.getString("font"), data.getString("format"),
        static_cast<unsigned int>(data.getInt("size")), data.getFloat("offsetX", 0.f), data.getFloat("offsetY", 0.f),
        DataReaders::readColors(data, "colors"));
}

void EngineComponentLoaders::loadStatusText(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::StatusText>(entity, data.getString("format"), readSlot(data), readPhase(data, "visibleWhen"));
}

} // namespace engine::scene
