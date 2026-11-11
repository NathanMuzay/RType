#include "IntRect.hpp"

namespace engine {

IntRect::IntRect(int x, int y, int width, int height) : m_x(x), m_y(y), m_width(width), m_height(height)
{
}

int IntRect::x() const
{
    return m_x;
}

int IntRect::y() const
{
    return m_y;
}

int IntRect::width() const
{
    return m_width;
}

int IntRect::height() const
{
    return m_height;
}

} // namespace engine
