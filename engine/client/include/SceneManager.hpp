#pragma once

#include <string>

#include "ComponentFactory.hpp"
#include "DataNode.hpp"
#include "IAudio.hpp"
#include "PrefabLibrary.hpp"
#include "Registry.hpp"
#include "SystemContext.hpp"
#include "SystemFactory.hpp"
#include "SystemManager.hpp"
#include "Vector2.hpp"

namespace engine::client {

class SceneManager
{
public:
    SceneManager(ecs::Registry &registry, scene::PrefabLibrary &prefabs, const scene::ComponentFactory &components,
        const scene::SystemFactory &systems, scene::SystemContext &context, audio::IAudio &audio, Vector2 area,
        scene::DataNode scenes);

    void switchTo(const std::string &name);
    void update(float dt);
    void clearSystems();
    void rebuildSystems();
    const std::string &current() const;
    std::string escapeAction() const;

private:
    void clearEntities();
    void playMusic(const scene::DataNode &scene);

    ecs::Registry &m_registry;
    scene::PrefabLibrary &m_prefabs;
    const scene::ComponentFactory &m_components;
    const scene::SystemFactory &m_systemFactory;
    scene::SystemContext &m_context;
    audio::IAudio &m_audio;
    Vector2 m_area;
    scene::DataNode m_scenes;
    ecs::SystemManager m_systems;
    std::string m_current;
    std::string m_music;
};

} // namespace engine::client
