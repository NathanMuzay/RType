#include "EngineSystemBuilders.hpp"

#include "AnchorSystem.hpp"
#include "AttachmentSystem.hpp"
#include "AudioSystem.hpp"
#include "BounceSystem.hpp"
#include "BoundsSystem.hpp"
#include "BoundTextSystem.hpp"
#include "ButtonStyleSystem.hpp"
#include "ChoiceSystem.hpp"
#include "CleanupSystem.hpp"
#include "CollisionSystem.hpp"
#include "Color.hpp"
#include "DamageSystem.hpp"
#include "DataReaders.hpp"
#include "DeathEffectSystem.hpp"
#include "FadeSystem.hpp"
#include "FocusNavigationSystem.hpp"
#include "GravitySystem.hpp"
#include "HitFlashSystem.hpp"
#include "InvulnerabilitySystem.hpp"
#include "KeyBindSystem.hpp"
#include "LifetimeSystem.hpp"
#include "ListViewSystem.hpp"
#include "LivesSystem.hpp"
#include "MatchSystem.hpp"
#include "MovementSystem.hpp"
#include "MusicSystem.hpp"
#include "OffscreenSystem.hpp"
#include "ParallaxSystem.hpp"
#include "ParticleSystem.hpp"
#include "PlayerControlSystem.hpp"
#include "PlayerLabelSystem.hpp"
#include "PointerSystem.hpp"
#include "ProgressBarSystem.hpp"
#include "RenderSystem.hpp"
#include "RespawnSystem.hpp"
#include "ScoreSystem.hpp"
#include "SettingSliderSystem.hpp"
#include "ShakeSystem.hpp"
#include "SolidSystem.hpp"
#include "SpriteAnimationSystem.hpp"
#include "StatusTextSystem.hpp"
#include "TextFieldSystem.hpp"
#include "TilemapSystem.hpp"
#include "TiltAnimationSystem.hpp"
#include "VisibilitySystem.hpp"
#include "WeaponSystem.hpp"

namespace engine::scene {

void EngineSystemBuilders::registerAll(SystemFactory &factory)
{
    registerSimulation(factory);
    registerRules(factory);
    registerPresentation(factory);
    registerInterface(factory);
}

void EngineSystemBuilders::registerInterface(SystemFactory &factory)
{
    factory.add("Pointer", &EngineSystemBuilders::buildUiActions<ecs::PointerSystem>);
    factory.add("FocusNavigation", &EngineSystemBuilders::buildUiActions<ecs::FocusNavigationSystem>);
    factory.add("BoundText", &EngineSystemBuilders::buildStoreReader<ecs::BoundTextSystem>);
    factory.add("Visibility", &EngineSystemBuilders::buildStoreReader<ecs::VisibilitySystem>);
    factory.add("ButtonStyle", &EngineSystemBuilders::buildButtonStyle);
    factory.add("TextField", &EngineSystemBuilders::buildTextField);
    factory.add("ListView", &EngineSystemBuilders::buildListView);
    factory.add("ProgressBar", &EngineSystemBuilders::buildStoreReader<ecs::ProgressBarSystem>);
    factory.add("Fade", &EngineSystemBuilders::buildStoreReader<ecs::FadeSystem>);
    factory.add("Choice", &EngineSystemBuilders::buildChoice);
    factory.add("KeyBind", &EngineSystemBuilders::buildKeyBind);
    factory.add("SettingSlider", &EngineSystemBuilders::buildSettingSlider);
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildButtonStyle(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::ButtonStyleSystem>(context.registry(), context.uiInput());
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildTextField(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::TextFieldSystem>(context.registry(), context.uiInput(), context.uiStore());
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildListView(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::ListViewSystem>(context.registry(), context.factory(), context.uiStore());
}

void EngineSystemBuilders::registerSimulation(SystemFactory &factory)
{
    factory.add("PlayerControl", &EngineSystemBuilders::buildSimple<ecs::PlayerControlSystem>);
    factory.add("Invulnerability", &EngineSystemBuilders::buildSimple<ecs::InvulnerabilitySystem>);
    factory.add("Movement", &EngineSystemBuilders::buildSimple<ecs::MovementSystem>);
    factory.add("Bounds", &EngineSystemBuilders::buildSimple<ecs::BoundsSystem>);
    factory.add("Offscreen", &EngineSystemBuilders::buildSimple<ecs::OffscreenSystem>);
    factory.add("Cleanup", &EngineSystemBuilders::buildSimple<ecs::CleanupSystem>);
    factory.add("Weapon", &EngineSystemBuilders::buildSpawning<ecs::WeaponSystem>);
    factory.add("Attachment", &EngineSystemBuilders::buildSimple<ecs::AttachmentSystem>);
    factory.add("HitFlash", &EngineSystemBuilders::buildSimple<ecs::HitFlashSystem>);
    factory.add("Solid", &EngineSystemBuilders::buildSimple<ecs::SolidSystem>);
    factory.add("Gravity", &EngineSystemBuilders::buildSimple<ecs::GravitySystem>);
    factory.add("Bounce", &EngineSystemBuilders::buildSimple<ecs::BounceSystem>);
    factory.add("Tilemap", &EngineSystemBuilders::buildTilemap);
    factory.add("Collision", &EngineSystemBuilders::buildCollision);
    factory.add("Damage", &EngineSystemBuilders::buildDamage);
}

void EngineSystemBuilders::registerRules(SystemFactory &factory)
{
    factory.add("Match", &EngineSystemBuilders::buildSimple<ecs::MatchSystem>);
    factory.add("Lives", &EngineSystemBuilders::buildDeathListener<ecs::LivesSystem>);
    factory.add("Score", &EngineSystemBuilders::buildDeathListener<ecs::ScoreSystem>);
    factory.add("Respawn", &EngineSystemBuilders::buildRespawn);
}

void EngineSystemBuilders::registerPresentation(SystemFactory &factory)
{
    factory.add("Parallax", &EngineSystemBuilders::buildParallax);
    factory.add("TiltAnimation", &EngineSystemBuilders::buildSimple<ecs::TiltAnimationSystem>);
    factory.add("SpriteAnimation", &EngineSystemBuilders::buildSimple<ecs::SpriteAnimationSystem>);
    factory.add("Anchor", &EngineSystemBuilders::buildSimple<ecs::AnchorSystem>);
    factory.add("StatusText", &EngineSystemBuilders::buildSimple<ecs::StatusTextSystem>);
    factory.add("DeathEffect", &EngineSystemBuilders::buildDeathEffect);
    factory.add("PlayerLabel", &EngineSystemBuilders::buildPlayerLabel);
    factory.add("Audio", &EngineSystemBuilders::buildAudio);
    factory.add("Render", &EngineSystemBuilders::buildRender);
    factory.add("Particles", &EngineSystemBuilders::buildSimple<ecs::ParticleSystem>);
    factory.add("Lifetime", &EngineSystemBuilders::buildSimple<ecs::LifetimeSystem>);
    factory.add("Music", &EngineSystemBuilders::buildMusic);
    factory.add("Shake", &EngineSystemBuilders::buildShake);
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildCollision(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::CollisionSystem>(context.registry(), context.collisions());
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildDamage(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::DamageSystem>(context.registry(), context.collisions(), context.deaths());
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildRespawn(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::RespawnSystem>(context.registry(), context.factory());
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildDeathEffect(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::DeathEffectSystem>(context.registry(), context.factory(), context.deaths());
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildPlayerLabel(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::PlayerLabelSystem>(context.registry(), context.flag("showPlayerLabels", true));
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildAudio(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::AudioSystem>(context.registry(), context.audio());
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildRender(SystemContext &context, const DataNode &entry)
{
    const Color background = entry.isString() ? Color(0, 0, 0) : DataReaders::readColor(entry, "background");
    return std::make_unique<ecs::RenderSystem>(context.registry(), context.renderer(), background);
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildTilemap(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::TilemapSystem>(
        context.registry(), context.factory(), context.canSpawn(), context.area());
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildMusic(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::MusicSystem>(context.registry(), context.uiStore(), context.audio());
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildParallax(SystemContext &context, const DataNode &entry)
{
    if (!entry.has("boostKey"))
        return std::make_unique<ecs::ParallaxSystem>(context.registry());
    return std::make_unique<ecs::ParallaxSystem>(context.registry(), context.uiStore(), entry.getString("boostKey"),
        entry.getString("boostValue"), entry.getFloat("boost"));
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildShake(SystemContext &context, const DataNode &entry)
{
    return std::make_unique<ecs::ShakeSystem>(
        context.uiStore(), context.renderer(), entry.getString("key"), entry.getFloat("strength", 1.f));
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildChoice(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::ChoiceSystem>(context.registry(), context.uiInput(), context.uiStore());
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildKeyBind(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::KeyBindSystem>(context.registry(), context.uiInput(), context.uiStore());
}

std::unique_ptr<ecs::ISystem> EngineSystemBuilders::buildSettingSlider(SystemContext &context, const DataNode &)
{
    return std::make_unique<ecs::SettingSliderSystem>(context.registry(), context.uiInput(), context.uiStore());
}

} // namespace engine::scene
