#include "Body.hpp"

namespace engine::physics {

Body::Body(std::size_t index, Aabb box) : m_index(index), m_box(box)
{
}

std::size_t Body::index() const
{
    return m_index;
}

const Aabb &Body::box() const
{
    return m_box;
}

} // namespace engine::physics
