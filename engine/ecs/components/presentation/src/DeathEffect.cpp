#include "DeathEffect.hpp"

#include <utility>

namespace engine::ecs {

DeathEffect::DeathEffect(std::string prefab) : m_prefab(std::move(prefab))
{
}

const std::string &DeathEffect::prefab() const
{
    return m_prefab;
}

} // namespace engine::ecs
