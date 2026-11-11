#include "SystemManager.hpp"

#include <utility>

#include "EcsError.hpp"

namespace engine::ecs {

void SystemManager::add(std::unique_ptr<ISystem> system)
{
    if (!system)
        throw EcsError("SystemManager: empty system");
    m_systems.push_back(std::move(system));
}

void SystemManager::update(float dt)
{
    if (dt < 0.f)
        throw EcsError("SystemManager: negative delta time");
    for (auto &system : m_systems)
        system->update(dt);
}

} // namespace engine::ecs
