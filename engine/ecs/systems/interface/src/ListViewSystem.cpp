#include "ListViewSystem.hpp"

#include <algorithm>
#include <string>
#include <utility>

#include "Button.hpp"
#include "ListView.hpp"
#include "RowData.hpp"
#include "Transform.hpp"

namespace engine::ecs {

ListViewSystem::ListViewSystem(Registry &registry, IEntityFactory &factory, const UiStore &store)
    : m_registry(registry), m_factory(factory), m_store(store)
{
}

void ListViewSystem::update(float)
{
    for (std::size_t i : m_registry.view<ListView, Transform>()) {
        const Entity list = m_registry.entityAt(i);
        const ListView &view = m_registry.get<ListView>(list);
        if (view.version() != m_store.listVersion(view.source()))
            rebuild(list);
    }
}

void ListViewSystem::rebuild(const Entity &list)
{
    const ListView view = m_registry.get<ListView>(list);
    const Transform origin = m_registry.get<Transform>(list);
    for (const Entity &row : view.rows())
        m_registry.kill(row);
    const std::vector<UiStore::Row> &rows = m_store.list(view.source());
    const std::size_t count = std::min(rows.size(), view.maxRows());
    std::vector<Entity> created;
    for (std::size_t index = 0; index < count; ++index) {
        const float rowY = origin.y() + static_cast<float>(index) * view.spacing();
        for (const ListRowPart &part : view.parts())
            created.push_back(spawnPart(part, origin.x() + part.offsetX(), rowY + part.offsetY(), rows[index]));
    }
    ListView &stored = m_registry.get<ListView>(list);
    stored.setRows(std::move(created));
    stored.setVersion(m_store.listVersion(view.source()));
}

Entity ListViewSystem::spawnPart(const ListRowPart &part, float x, float y, const UiStore::Row &row)
{
    const Entity entity = m_factory.spawn(part.prefab());
    m_registry.emplace<Transform>(entity, x, y);
    m_registry.emplace<RowData>(entity, row);
    if (m_registry.has<Button>(entity)) {
        Button &button = m_registry.get<Button>(entity);
        button.setAction(m_store.format(button.action(), &row));
    }
    return entity;
}

} // namespace engine::ecs
