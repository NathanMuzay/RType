#pragma once

#include <cstddef>

#include "IComponent.hpp"

namespace engine::ecs {

class PlayerIndex : public IComponent
{
public:
    explicit PlayerIndex(std::size_t slot);

    std::size_t slot() const;

private:
    std::size_t m_slot;
};

} // namespace engine::ecs
