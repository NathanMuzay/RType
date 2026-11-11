#pragma once

#include <cstdint>

#include "IComponent.hpp"

namespace engine::ecs {

class Score : public IComponent
{
public:
    Score();

    std::uint32_t points() const;
    void setPoints(std::uint32_t points);
    void add(std::uint32_t points);
    void reset();

private:
    std::uint32_t m_points;
};

} // namespace engine::ecs
