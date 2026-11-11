#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "DataNode.hpp"
#include "Entity.hpp"
#include "Registry.hpp"

namespace engine::scene {

class ComponentFactory
{
public:
    using Loader = std::function<void(ecs::Registry &, const ecs::Entity &, const DataNode &)>;

    void add(const std::string &name, Loader loader);
    bool has(const std::string &name) const;
    void apply(ecs::Registry &registry, const ecs::Entity &entity, const std::string &name, const DataNode &data) const;

private:
    std::unordered_map<std::string, Loader> m_loaders;
};

} // namespace engine::scene
