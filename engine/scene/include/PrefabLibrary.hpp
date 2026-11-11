#pragma once

#include <string>
#include <unordered_map>

#include "ComponentFactory.hpp"
#include "DataNode.hpp"
#include "IEntityFactory.hpp"
#include "Registry.hpp"

namespace engine::scene {

class PrefabLibrary : public ecs::IEntityFactory
{
public:
    PrefabLibrary(ecs::Registry &registry, const ComponentFactory &factory);

    void load(const DataNode &prefabs);
    bool has(const std::string &name) const;
    ecs::Entity spawn(const std::string &name) override;

private:
    ecs::Registry &m_registry;
    const ComponentFactory &m_factory;
    std::unordered_map<std::string, DataNode> m_prefabs;
};

} // namespace engine::scene
