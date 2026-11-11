#pragma once

#include <cstddef>

#include "Aabb.hpp"

namespace engine::physics {

class Body
{
public:
    Body(std::size_t index, Aabb box);

    std::size_t index() const;
    const Aabb &box() const;

private:
    std::size_t m_index;
    Aabb m_box;
};

} // namespace engine::physics
