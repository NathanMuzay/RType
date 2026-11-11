#pragma once

#include <cstddef>

namespace engine::physics {

class CollisionPair
{
public:
    CollisionPair(std::size_t first, std::size_t second);

    std::size_t first() const;
    std::size_t second() const;

private:
    std::size_t m_first;
    std::size_t m_second;
};

} // namespace engine::physics
