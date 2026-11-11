#include "TilemapSystem.hpp"

#include <cstddef>
#include <string>
#include <utility>

#include "Tilemap.hpp"
#include "Transform.hpp"
#include "Velocity.hpp"

namespace engine::ecs {

TilemapSystem::TilemapSystem(Registry &registry, IEntityFactory &factory, std::function<bool()> canSpawn, Vector2 area)
    : m_registry(registry), m_factory(factory), m_canSpawn(std::move(canSpawn)), m_area(area)
{
}

void TilemapSystem::update(float dt)
{
    for (std::size_t i : m_registry.view<Tilemap, Transform>()) {
        const Entity map = m_registry.entityAt(i);
        Tilemap &tiles = m_registry.get<Tilemap>(map);
        tiles.scroll(tiles.speed() * dt);
        const float originX = m_registry.get<Transform>(map).x();
        while (tiles.nextColumn() < tiles.columns()) {
            const float x = originX + static_cast<float>(tiles.nextColumn()) * tiles.tileSize() - tiles.scrolled();
            if (x > m_area.x() + tiles.tileSize() || !m_canSpawn())
                break;
            spawnColumn(map, x);
            m_registry.get<Tilemap>(map).advanceColumn();
        }
    }
}

void TilemapSystem::spawnColumn(const Entity &map, float x)
{
    const Tilemap tiles = m_registry.get<Tilemap>(map);
    const float originY = m_registry.get<Transform>(map).y();
    for (std::size_t row = 0; row < tiles.rows().size(); ++row) {
        const std::string &line = tiles.rows()[row];
        if (tiles.nextColumn() >= line.size())
            continue;
        const auto prefab = tiles.legend().find(line[tiles.nextColumn()]);
        if (prefab == tiles.legend().end())
            continue;
        const Entity tile = m_factory.spawn(prefab->second);
        const float y = originY + static_cast<float>(row) * tiles.tileSize();
        m_registry.emplace<Transform>(tile, x, y);
        m_registry.emplace<Velocity>(tile, -tiles.speed(), 0.f);
    }
}

} // namespace engine::ecs
