#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "DataNode.hpp"
#include "ISystem.hpp"
#include "SystemContext.hpp"
#include "SystemManager.hpp"

namespace engine::scene {

class SystemFactory
{
public:
    using Builder = std::function<std::unique_ptr<ecs::ISystem>(SystemContext &, const DataNode &)>;

    void add(const std::string &name, Builder builder);
    bool has(const std::string &name) const;
    void remove(const std::string &name);
    std::unique_ptr<ecs::ISystem> create(SystemContext &context, const DataNode &entry) const;
    void build(SystemContext &context, const DataNode &list, ecs::SystemManager &systems) const;

private:
    std::unordered_map<std::string, Builder> m_builders;
};

} // namespace engine::scene
