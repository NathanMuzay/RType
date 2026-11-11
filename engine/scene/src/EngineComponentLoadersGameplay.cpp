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

void EngineComponentLoaders::loadTransform(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Transform>(entity, data.getFloat("x", 0.f), data.getFloat("y", 0.f));
}

void EngineComponentLoaders::loadVelocity(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Velocity>(entity, data.getFloat("x", 0.f), data.getFloat("y", 0.f));
}

void EngineComponentLoaders::loadCollider(
    const LayerTable &layers, ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    const std::uint32_t hits = data.has("hits") ? layers.mask(data.child("hits")) : 0;
    registry.emplace<ecs::Collider>(
        entity, data.getFloat("width"), data.getFloat("height"), layers.bit(data.getString("layer")), hits);
}

void EngineComponentLoaders::loadBounded(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Bounded>(
        entity, data.getFloat("minX"), data.getFloat("minY"), data.getFloat("maxX"), data.getFloat("maxY"));
}

void EngineComponentLoaders::loadHealth(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Health>(entity, data.getInt("points"));
}

void EngineComponentLoaders::loadContactDamage(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::ContactDamage>(entity, data.getInt("amount"), data.getBool("destroyOnHit", false));
}

void EngineComponentLoaders::loadInvulnerable(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Invulnerable>(entity, data.getFloat("seconds"));
}

void EngineComponentLoaders::loadDespawnOffscreen(
    ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::DespawnOffscreen>(
        entity, data.getFloat("minX"), data.getFloat("minY"), data.getFloat("maxX"), data.getFloat("maxY"));
}

void EngineComponentLoaders::loadPlayerControl(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::PlayerControl>(entity, data.getFloat("speed"));
}

void EngineComponentLoaders::loadScoreValue(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::ScoreValue>(entity, static_cast<std::uint32_t>(data.getInt("points")));
}

void EngineComponentLoaders::loadWeapon(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Weapon>(entity, data.getString("prefab"), data.getFloat("cooldown"),
        data.getFloat("offsetX", 0.f), data.getFloat("offsetY", 0.f), data.getInt("count", 1),
        data.getFloat("spread", 0.f));
}

void EngineComponentLoaders::loadScript(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    ecs::Script::Numbers numbers;
    ecs::Script::Texts texts;
    if (data.has("params")) {
        const DataNode params = data.child("params");
        for (const std::string &name : params.keys()) {
            const DataNode value = params.child(name);
            if (value.isString())
                texts.emplace(name, value.asString());
            else
                numbers.emplace(name, value.asFloat());
        }
    }
    registry.emplace<ecs::Script>(entity, data.getString("file"), std::move(numbers), std::move(texts));
}

void EngineComponentLoaders::loadSolid(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &)
{
    registry.emplace<ecs::Solid>(entity);
}

void EngineComponentLoaders::loadGravity(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Gravity>(entity, data.getFloat("force"));
}

void EngineComponentLoaders::loadBounce(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Bounce>(entity, data.getFloat("restitution", 1.f));
}

void EngineComponentLoaders::loadPushable(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &)
{
    registry.emplace<ecs::Pushable>(entity);
}

void EngineComponentLoaders::loadTilemap(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    std::vector<std::string> rows;
    for (const DataNode &row : data.child("rows").items())
        rows.push_back(row.asString());
    ecs::Tilemap::Legend legend;
    const DataNode tiles = data.child("legend");
    for (const std::string &symbol : tiles.keys()) {
        if (symbol.size() != 1)
            throw SceneError("Tilemap: legend keys must be single characters, got " + symbol);
        legend.emplace(symbol[0], tiles.getString(symbol));
    }
    registry.emplace<ecs::Tilemap>(
        entity, std::move(rows), std::move(legend), data.getFloat("tileSize"), data.getFloat("speed"));
}

void EngineComponentLoaders::loadRotation(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Rotation>(entity, data.getFloat("degrees", 0.f));
}

void EngineComponentLoaders::loadReplicated(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Replicated>(entity, static_cast<std::uint8_t>(data.getInt("kind")));
}

void EngineComponentLoaders::loadLives(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Lives>(entity, data.getInt("count"));
}

void EngineComponentLoaders::loadScore(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &)
{
    registry.emplace<ecs::Score>(entity);
}

void EngineComponentLoaders::loadRespawn(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Respawn>(entity, data.getString("prefab"), data.getFloat("delay", 0.f),
        data.getFloat("invulnerability", 0.f), data.getFloat("x"), data.getFloat("y"), data.getFloat("spacingX", 0.f),
        data.getFloat("spacingY", 0.f));
}

void EngineComponentLoaders::loadMatch(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data)
{
    registry.emplace<ecs::Match>(entity, data.getFloat("restartDelay", 0.f));
}

void EngineComponentLoaders::loadPersistent(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &)
{
    registry.emplace<ecs::Persistent>(entity);
}

} // namespace engine::scene
