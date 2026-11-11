#include "Lives.hpp"

namespace engine::ecs {

Lives::Lives(int count) : m_start(count), m_count(count)
{
}

int Lives::count() const
{
    return m_count;
}

void Lives::setCount(int count)
{
    m_count = count;
}

void Lives::lose()
{
    if (m_count > 0)
        --m_count;
}

void Lives::reset()
{
    m_count = m_start;
}

} // namespace engine::ecs
