#pragma once

#include <string>

#include "Color.hpp"
#include "IntRect.hpp"
#include "Vector2.hpp"

namespace engine::render {

class IRenderer
{
public:
    virtual ~IRenderer() = default;

    virtual void clear(Color color) = 0;
    virtual void drawRect(Vector2 position, Vector2 size, Color color, Color outline, float thickness) = 0;
    virtual void drawTexture(
        const std::string &texture, IntRect source, Vector2 position, Vector2 scale, float rotation, Color tint) = 0;
    virtual void setOffset(Vector2 offset) = 0;
    virtual Vector2 offset() const = 0;
    virtual void drawText(
        const std::string &font, const std::string &text, unsigned int size, Vector2 position, Color color) = 0;
    virtual void display() = 0;
};

} // namespace engine::render
