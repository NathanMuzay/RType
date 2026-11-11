#include "SceneManager.hpp"

#include <utility>

#include "ClientError.hpp"
#include "Persistent.hpp"
#include "SceneBuilder.hpp"
#include "Transform.hpp"

namespace engine::client {

SceneManager::SceneManager(ecs::Registry &registry, scene::PrefabLibrary &prefabs,
    const scene::ComponentFactory &components, const scene::SystemFactory &systems, scene::SystemContext &context,
    audio::IAudio &audio, Vector2 area, scene::DataNode scenes)
    : m_registry(registry), m_prefabs(prefabs), m_components(components), m_systemFactory(systems), m_context(context),
      m_audio(audio), m_area(area), m_scenes(std::move(scenes)), m_systems(), m_current(), m_music()
{
}

void SceneManager::switchTo(const std::string &name)
{
    if (!m_scenes.has(name))
        throw ClientError("SceneManager: unknown scene " + name);
    const scene::DataNode scene = m_scenes.child(name);
    clearEntities();
    m_context.uiActions().clear();
    m_systems = ecs::SystemManager();
    m_systemFactory.build(m_context, scene.child("systems"), m_systems);
    scene::SceneBuilder(m_registry, m_prefabs, m_components, m_area).build(scene.child("entities"));
    playMusic(scene);
    m_current = name;
}

void SceneManager::update(float dt)
{
    m_systems.update(dt);
}

void SceneManager::clearSystems()
{
    m_systems = ecs::SystemManager();
}

void SceneManager::rebuildSystems()
{
    m_systems = ecs::SystemManager();
    if (!m_current.empty())
        m_systemFactory.build(m_context, m_scenes.child(m_current).child("systems"), m_systems);
}

const std::string &SceneManager::current() const
{
    return m_current;
}

std::string SceneManager::escapeAction() const
{
    if (m_current.empty())
        return {};
    const scene::DataNode scene = m_scenes.child(m_current);
    return scene.has("escapeAction") ? scene.getString("escapeAction") : std::string();
}

void SceneManager::clearEntities()
{
    for (std::size_t i : m_registry.view<ecs::Transform>()) {
        const ecs::Entity entity = m_registry.entityAt(i);
        if (!m_registry.has<ecs::Persistent>(entity))
            m_registry.kill(entity);
    }
}

void SceneManager::playMusic(const scene::DataNode &scene)
{
    const std::string music = scene.has("music") ? scene.getString("music") : std::string();
    if (music == m_music)
        return;
    if (music.empty())
        m_audio.stopMusic();
    else
        m_audio.playMusic(music);
    m_music = music;
}

} // namespace engine::client
