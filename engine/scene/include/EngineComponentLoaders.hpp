#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "ComponentFactory.hpp"
#include "DataNode.hpp"
#include "Entity.hpp"
#include "LayerTable.hpp"
#include "Match.hpp"
#include "Registry.hpp"

namespace engine::scene {

class EngineComponentLoaders
{
public:
    static void registerAll(ComponentFactory &factory, const LayerTable &layers);

private:
    static void registerGameplay(ComponentFactory &factory, const LayerTable &layers);
    static void registerRules(ComponentFactory &factory);
    static void registerPresentation(ComponentFactory &factory);
    static void registerInterface(ComponentFactory &factory);

    static void loadTransform(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadVelocity(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadCollider(
        const LayerTable &layers, ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadBounded(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadHealth(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadContactDamage(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadInvulnerable(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadDespawnOffscreen(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadPlayerControl(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadScoreValue(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadWeapon(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadReplicated(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadScript(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadSolid(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadGravity(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadBounce(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadPushable(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadTilemap(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadParticleEmitter(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadLifetime(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadMusicWhen(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadChoice(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadKeyBind(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadSettingSlider(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static std::vector<std::string> sliderRange(const DataNode &data);
    static void loadRotation(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);

    static void loadLives(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadScore(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadRespawn(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadMatch(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadPersistent(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);

    static void loadSprite(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadRectShape(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadParallax(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadTiltAnimation(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadSpriteAnimation(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadSoundCue(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadText(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadDeathEffect(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadPlayerLabel(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadStatusText(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);

    static void loadButton(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadFocusable(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadTextField(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadVisibleWhen(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadBoundText(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadListView(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadHidden(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadBackground(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadForeground(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadProgressBar(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static void loadFadeWhen(ecs::Registry &registry, const ecs::Entity &entity, const DataNode &data);
    static std::string readOptionalString(const DataNode &data, const std::string &key);

    static std::optional<ecs::Match::Phase> readPhase(const DataNode &data, const std::string &key);
    static std::optional<std::size_t> readSlot(const DataNode &data);
};

} // namespace engine::scene
