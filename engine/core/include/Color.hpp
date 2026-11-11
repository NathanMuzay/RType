#pragma once

#include <cstdint>

namespace engine {

class Color
{
public:
    Color(std::uint8_t r = 255, std::uint8_t g = 255, std::uint8_t b = 255, std::uint8_t a = 255);

    std::uint8_t r() const;
    std::uint8_t g() const;
    std::uint8_t b() const;
    std::uint8_t a() const;

private:
    std::uint8_t m_r;
    std::uint8_t m_g;
    std::uint8_t m_b;
    std::uint8_t m_a;
};

} // namespace engine
