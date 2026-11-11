#include "Replicated.hpp"

namespace engine::ecs {

Replicated::Replicated(std::uint8_t kind) : m_kind(kind), m_networkId(0)
{
}

std::uint8_t Replicated::kind() const
{
    return m_kind;
}

std::uint16_t Replicated::networkId() const
{
    return m_networkId;
}

void Replicated::setNetworkId(std::uint16_t networkId)
{
    m_networkId = networkId;
}

} // namespace engine::ecs
