#pragma once

#include <vector>

#include "Entity.hpp"
#include "IEntityFactory.hpp"
#include "ISystem.hpp"
#include "ListRowPart.hpp"
#include "Registry.hpp"
#include "UiStore.hpp"

namespace engine::ecs {

class ListViewSystem : public ISystem
{
public:
    ListViewSystem(Registry &registry, IEntityFactory &factory, const UiStore &store);

    void update(float dt) override;

private:
    void rebuild(const Entity &list);
    Entity spawnPart(const ListRowPart &part, float x, float y, const UiStore::Row &row);

    Registry &m_registry;
    IEntityFactory &m_factory;
    const UiStore &m_store;
};

} // namespace engine::ecs
