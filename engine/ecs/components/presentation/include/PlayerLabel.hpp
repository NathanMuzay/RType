#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Color.hpp"
#include "IComponent.hpp"

namespace engine::ecs {

class PlayerLabel : public IComponent
{
public:
    PlayerLabel(std::string font, std::string format, unsigned int size, float offsetX, float offsetY,
        std::vector<Color> colors);

    const std::string &font() const;
    const std::string &format() const;
    unsigned int size() const;
    float offsetX() const;
    float offsetY() const;
    Color colorFor(std::size_t slot) const;
    bool attached() const;
    void markAttached();

private:
    std::string m_font;
    std::string m_format;
    unsigned int m_size;
    float m_offsetX;
    float m_offsetY;
    std::vector<Color> m_colors;
    bool m_attached;
};

} // namespace engine::ecs
