#pragma once

#include <cstdint>

#include "IComponent.hpp"

namespace engine::ecs {

class ScoreValue : public IComponent
{
public:
    explicit ScoreValue(std::uint32_t points);

    std::uint32_t points() const;

private:
    std::uint32_t m_points;
};

} // namespace engine::ecs
