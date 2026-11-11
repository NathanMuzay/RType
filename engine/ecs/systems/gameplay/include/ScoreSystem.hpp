#pragma once

#include <cstddef>
#include <cstdint>

#include "DeathEvent.hpp"
#include "Entity.hpp"
#include "EventQueue.hpp"
#include "ISystem.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class ScoreSystem : public ISystem
{
public:
    static constexpr std::size_t MAX_OWNER_DEPTH = 8;

    ScoreSystem(Registry &registry, const EventQueue<DeathEvent> &deaths);

    void update(float dt) override;

private:
    void credit(Entity killer, std::uint32_t points);

    Registry &m_registry;
    const EventQueue<DeathEvent> &m_deaths;
};

} // namespace engine::ecs
