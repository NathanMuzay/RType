#include "Tilemap.hpp"

#include <algorithm>
#include <utility>

namespace engine::ecs {

Tilemap::Tilemap(std::vector<std::string> rows, Legend legend, float tileSize, float speed)
    : m_rows(std::move(rows)), m_legend(std::move(legend)), m_tileSize(tileSize), m_speed(speed), m_nextColumn(0),
      m_scrolled(0.f)
{
}

const std::vector<std::string> &Tilemap::rows() const
{
    return m_rows;
}

const Tilemap::Legend &Tilemap::legend() const
{
    return m_legend;
}

float Tilemap::tileSize() const
{
    return m_tileSize;
}

float Tilemap::speed() const
{
    return m_speed;
}

std::size_t Tilemap::columns() const
{
    std::size_t widest = 0;
    for (const std::string &row : m_rows)
        widest = std::max(widest, row.size());
    return widest;
}

std::size_t Tilemap::nextColumn() const
{
    return m_nextColumn;
}

void Tilemap::advanceColumn()
{
    ++m_nextColumn;
}

float Tilemap::scrolled() const
{
    return m_scrolled;
}

void Tilemap::scroll(float distance)
{
    m_scrolled += distance;
}

} // namespace engine::ecs
