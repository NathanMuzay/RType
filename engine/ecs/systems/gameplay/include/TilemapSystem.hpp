#pragma once

#include <functional>

#include "IEntityFactory.hpp"
#include "ISystem.hpp"
#include "Registry.hpp"
#include "Vector2.hpp"

namespace engine::ecs {

class TilemapSystem : public ISystem
{
public:
    TilemapSystem(Registry &registry, IEntityFactory &factory, std::function<bool()> canSpawn, Vector2 area);

    void update(float dt) override;

private:
    void spawnColumn(const Entity &map, float x);

    Registry &m_registry;
    IEntityFactory &m_factory;
    std::function<bool()> m_canSpawn;
    Vector2 m_area;
};

} // namespace engine::ecs
