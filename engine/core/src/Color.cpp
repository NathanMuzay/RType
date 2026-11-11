#include "Color.hpp"

namespace engine {

Color::Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) : m_r(r), m_g(g), m_b(b), m_a(a)
{
}

std::uint8_t Color::r() const
{
    return m_r;
}

std::uint8_t Color::g() const
{
    return m_g;
}

std::uint8_t Color::b() const
{
    return m_b;
}

std::uint8_t Color::a() const
{
    return m_a;
}

} // namespace engine
