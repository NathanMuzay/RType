#pragma once

#include <string>

#include "IComponent.hpp"

namespace engine::ecs {

class DeathEffect : public IComponent
{
public:
    explicit DeathEffect(std::string prefab);

    const std::string &prefab() const;

private:
    std::string m_prefab;
};

} // namespace engine::ecs
