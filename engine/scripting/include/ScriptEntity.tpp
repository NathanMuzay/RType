#pragma once

#include "ScriptWorld.hpp"

namespace engine::scripting {

template <class Component>
const Component *ScriptEntity::component() const
{
    const ecs::Registry &registry = m_world->registry();
    if (!registry.isAlive(m_entity) || !registry.has<Component>(m_entity))
        return nullptr;
    return &registry.get<Component>(m_entity);
}

template <class Component>
Component *ScriptEntity::liveComponent()
{
    if (!alive() || !m_world->registry().has<Component>(m_entity))
        return nullptr;
    return &m_world->registry().get<Component>(m_entity);
}

} // namespace engine::scripting
