#pragma once

#include "ISystem.hpp"
#include "Match.hpp"
#include "Registry.hpp"

namespace engine::ecs {

class MatchSystem : public ISystem
{
public:
    explicit MatchSystem(Registry &registry);

    void update(float dt) override;

    static Match::Phase phaseOf(const Registry &registry);

private:
    bool everyoneOut() const;
    void clearWorld();
    void resetPlayers();

    Registry &m_registry;
};

} // namespace engine::ecs
