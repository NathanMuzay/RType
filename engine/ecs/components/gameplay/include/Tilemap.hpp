#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "IComponent.hpp"

namespace engine::ecs {

class Tilemap : public IComponent
{
public:
    using Legend = std::unordered_map<char, std::string>;

    Tilemap(std::vector<std::string> rows, Legend legend, float tileSize, float speed);

    const std::vector<std::string> &rows() const;
    const Legend &legend() const;
    float tileSize() const;
    float speed() const;
    std::size_t columns() const;
    std::size_t nextColumn() const;
    void advanceColumn();
    float scrolled() const;
    void scroll(float distance);

private:
    std::vector<std::string> m_rows;
    Legend m_legend;
    float m_tileSize;
    float m_speed;
    std::size_t m_nextColumn;
    float m_scrolled;
};

} // namespace engine::ecs
