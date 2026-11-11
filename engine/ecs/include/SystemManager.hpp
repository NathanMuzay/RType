#pragma once

#include <memory>
#include <vector>

#include "ISystem.hpp"

namespace engine::ecs {

class SystemManager
{
public:
    void add(std::unique_ptr<ISystem> system);
    void update(float dt);

private:
    std::vector<std::unique_ptr<ISystem>> m_systems;
};

} // namespace engine::ecs
