#pragma once

#include <memory>

#include "DataNode.hpp"
#include "ISystem.hpp"
#include "SystemContext.hpp"
#include "SystemFactory.hpp"

namespace engine::scene {

class EngineSystemBuilders
{
public:
    static void registerAll(SystemFactory &factory);

private:
    static void registerSimulation(SystemFactory &factory);
    static void registerRules(SystemFactory &factory);
    static void registerPresentation(SystemFactory &factory);
    static void registerInterface(SystemFactory &factory);

    template <class System>
    static std::unique_ptr<ecs::ISystem> buildSimple(SystemContext &context, const DataNode &entry);
    template <class System>
    static std::unique_ptr<ecs::ISystem> buildUiActions(SystemContext &context, const DataNode &entry);
    template <class System>
    static std::unique_ptr<ecs::ISystem> buildStoreReader(SystemContext &context, const DataNode &entry);
    template <class System>
    static std::unique_ptr<ecs::ISystem> buildSpawning(SystemContext &context, const DataNode &entry);
    template <class System>
    static std::unique_ptr<ecs::ISystem> buildDeathListener(SystemContext &context, const DataNode &entry);

    static std::unique_ptr<ecs::ISystem> buildCollision(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildDamage(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildRespawn(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildDeathEffect(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildPlayerLabel(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildAudio(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildRender(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildButtonStyle(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildTextField(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildListView(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildTilemap(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildMusic(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildParallax(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildShake(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildChoice(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildKeyBind(SystemContext &context, const DataNode &entry);
    static std::unique_ptr<ecs::ISystem> buildSettingSlider(SystemContext &context, const DataNode &entry);
};

} // namespace engine::scene

#include "EngineSystemBuilders.tpp"
