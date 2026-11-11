#include "EngineComponentLoaders.hpp"

#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

#include "Background.hpp"
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

void EngineComponentLoaders::registerAll(ComponentFactory &factory, const LayerTable &layers)
{
    registerGameplay(factory, layers);
    registerRules(factory);
    registerPresentation(factory);
    registerInterface(factory);
}

void EngineComponentLoaders::registerInterface(ComponentFactory &factory)
{
    factory.add("Button", &EngineComponentLoaders::loadButton);
    factory.add("Focusable", &EngineComponentLoaders::loadFocusable);
    factory.add("TextField", &EngineComponentLoaders::loadTextField);
    factory.add("VisibleWhen", &EngineComponentLoaders::loadVisibleWhen);
    factory.add("BoundText", &EngineComponentLoaders::loadBoundText);
    factory.add("ListView", &EngineComponentLoaders::loadListView);
    factory.add("Hidden", &EngineComponentLoaders::loadHidden);
    factory.add("Background", &EngineComponentLoaders::loadBackground);
    factory.add("Foreground", &EngineComponentLoaders::loadForeground);
    factory.add("ProgressBar", &EngineComponentLoaders::loadProgressBar);
    factory.add("FadeWhen", &EngineComponentLoaders::loadFadeWhen);
    factory.add("KeyBind", &EngineComponentLoaders::loadKeyBind);
    factory.add("SettingSlider", &EngineComponentLoaders::loadSettingSlider);
}

std::string EngineComponentLoaders::readOptionalString(const DataNode &data, const std::string &key)
{
    return data.has(key) ? data.getString(key) : std::string();
}

void EngineComponentLoaders::registerGameplay(ComponentFactory &factory, const LayerTable &layers)
{
    factory.add("Transform", &EngineComponentLoaders::loadTransform);
    factory.add("Velocity", &EngineComponentLoaders::loadVelocity);
    factory.add("Collider", std::bind_front(&EngineComponentLoaders::loadCollider, std::cref(layers)));
    factory.add("Bounded", &EngineComponentLoaders::loadBounded);
    factory.add("Health", &EngineComponentLoaders::loadHealth);
    factory.add("ContactDamage", &EngineComponentLoaders::loadContactDamage);
    factory.add("Invulnerable", &EngineComponentLoaders::loadInvulnerable);
    factory.add("DespawnOffscreen", &EngineComponentLoaders::loadDespawnOffscreen);
    factory.add("PlayerControl", &EngineComponentLoaders::loadPlayerControl);
    factory.add("ScoreValue", &EngineComponentLoaders::loadScoreValue);
    factory.add("Weapon", &EngineComponentLoaders::loadWeapon);
    factory.add("Replicated", &EngineComponentLoaders::loadReplicated);
    factory.add("Script", &EngineComponentLoaders::loadScript);
    factory.add("Solid", &EngineComponentLoaders::loadSolid);
    factory.add("Gravity", &EngineComponentLoaders::loadGravity);
    factory.add("Bounce", &EngineComponentLoaders::loadBounce);
    factory.add("Pushable", &EngineComponentLoaders::loadPushable);
    factory.add("Tilemap", &EngineComponentLoaders::loadTilemap);
    factory.add("ParticleEmitter", &EngineComponentLoaders::loadParticleEmitter);
    factory.add("Lifetime", &EngineComponentLoaders::loadLifetime);
    factory.add("MusicWhen", &EngineComponentLoaders::loadMusicWhen);
    factory.add("Choice", &EngineComponentLoaders::loadChoice);
    factory.add("Rotation", &EngineComponentLoaders::loadRotation);
}

void EngineComponentLoaders::registerRules(ComponentFactory &factory)
{
    factory.add("Lives", &EngineComponentLoaders::loadLives);
    factory.add("Score", &EngineComponentLoaders::loadScore);
    factory.add("Respawn", &EngineComponentLoaders::loadRespawn);
    factory.add("Match", &EngineComponentLoaders::loadMatch);
    factory.add("Persistent", &EngineComponentLoaders::loadPersistent);
}

void EngineComponentLoaders::registerPresentation(ComponentFactory &factory)
{
    factory.add("Sprite", &EngineComponentLoaders::loadSprite);
    factory.add("RectShape", &EngineComponentLoaders::loadRectShape);
    factory.add("Parallax", &EngineComponentLoaders::loadParallax);
    factory.add("TiltAnimation", &EngineComponentLoaders::loadTiltAnimation);
    factory.add("SpriteAnimation", &EngineComponentLoaders::loadSpriteAnimation);
    factory.add("SoundCue", &EngineComponentLoaders::loadSoundCue);
    factory.add("Text", &EngineComponentLoaders::loadText);
    factory.add("DeathEffect", &EngineComponentLoaders::loadDeathEffect);
    factory.add("PlayerLabel", &EngineComponentLoaders::loadPlayerLabel);
    factory.add("StatusText", &EngineComponentLoaders::loadStatusText);
}

std::optional<ecs::Match::Phase> EngineComponentLoaders::readPhase(const DataNode &data, const std::string &key)
{
    if (!data.has(key))
        return std::nullopt;
    const std::string phase = data.getString(key);
    if (phase == "waiting")
        return ecs::Match::Phase::Waiting;
    if (phase == "playing")
        return ecs::Match::Phase::Playing;
    if (phase == "over")
        return ecs::Match::Phase::Over;
    throw SceneError("EngineComponentLoaders: unknown match phase " + phase + " in " + data.path());
}

std::optional<std::size_t> EngineComponentLoaders::readSlot(const DataNode &data)
{
    if (!data.has("slot"))
        return std::nullopt;
    const int slot = data.getInt("slot");
    if (slot < 0)
        throw SceneError("EngineComponentLoaders: " + data.path() + ".slot must not be negative");
    return static_cast<std::size_t>(slot);
}

} // namespace engine::scene
