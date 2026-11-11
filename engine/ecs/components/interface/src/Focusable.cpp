#include "Focusable.hpp"

namespace engine::ecs {

Focusable::Focusable(int order) : m_order(order)
{
}

int Focusable::order() const
{
    return m_order;
}

} // namespace engine::ecs
