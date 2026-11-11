#include "BoundTextSystem.hpp"

#include "BoundText.hpp"
#include "RowData.hpp"
#include "Text.hpp"

namespace engine::ecs {

BoundTextSystem::BoundTextSystem(Registry &registry, const UiStore &store) : m_registry(registry), m_store(store)
{
}

void BoundTextSystem::update(float)
{
    const auto &bindings = m_registry.getComponents<BoundText>();
    auto &texts = m_registry.getComponents<Text>();
    for (std::size_t i : m_registry.view<BoundText, Text>()) {
        const Entity entity = m_registry.entityAt(i);
        const UiStore::Row *row = m_registry.has<RowData>(entity) ? &m_registry.get<RowData>(entity).values() : nullptr;
        texts.at(i).setContent(m_store.format(bindings.at(i).format(), row));
    }
}

} // namespace engine::ecs
