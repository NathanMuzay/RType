#pragma once

#include <cstdint>

#include "IComponent.hpp"

namespace engine::ecs {

class Replicated : public IComponent
{
public:
    explicit Replicated(std::uint8_t kind);

    std::uint8_t kind() const;
    std::uint16_t networkId() const;
    void setNetworkId(std::uint16_t networkId);

private:
    std::uint8_t m_kind;
    std::uint16_t m_networkId;
};

} // namespace engine::ecs
