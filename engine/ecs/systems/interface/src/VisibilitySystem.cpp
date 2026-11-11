#include "VisibilitySystem.hpp"

#include "Hidden.hpp"
#include "Hovered.hpp"
#include "RowData.hpp"
#include "VisibleWhen.hpp"

namespace engine::ecs {

VisibilitySystem::VisibilitySystem(Registry &registry, const UiStore &store) : m_registry(registry), m_store(store)
{
}

void VisibilitySystem::update(float)
{
    for (std::size_t i : m_registry.view<VisibleWhen>()) {
        const Entity entity = m_registry.entityAt(i);
        const VisibleWhen &rule = m_registry.get<VisibleWhen>(entity);
        const UiStore::Row *row = m_registry.has<RowData>(entity) ? &m_registry.get<RowData>(entity).values() : nullptr;
        const bool blocked = !rule.unlessKey().empty() && m_store.lookup(rule.unlessKey(), row) == rule.unlessValue();
        bool visible = !blocked && m_store.lookup(rule.key(), row) == rule.value();
        for (const VisibleWhen::Condition &condition : rule.also())
            visible = visible && m_store.lookup(condition.first, row) == condition.second;
        if (visible && m_registry.has<Hidden>(entity)) {
            m_registry.remove<Hidden>(entity);
        } else if (!visible && !m_registry.has<Hidden>(entity)) {
            m_registry.emplace<Hidden>(entity);
            if (m_registry.has<Hovered>(entity))
                m_registry.remove<Hovered>(entity);
        }
    }
}

} // namespace engine::ecs
