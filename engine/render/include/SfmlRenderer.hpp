#pragma once

#include <string>

#include "FontManager.hpp"
#include "IRenderer.hpp"
#include "TextureManager.hpp"
#include "Window.hpp"

namespace engine::render {

class SfmlRenderer : public IRenderer
{
public:
    SfmlRenderer(Window &window, const TextureManager &textures, const FontManager &fonts);

    void setTextScale(float scale);

    void clear(Color color) override;
    void drawRect(Vector2 position, Vector2 size, Color color, Color outline, float thickness) override;
    void drawTexture(const std::string &texture, IntRect source, Vector2 position, Vector2 scale, float rotation,
        Color tint) override;
    void setOffset(Vector2 offset) override;
    Vector2 offset() const override;
    void drawText(
        const std::string &font, const std::string &text, unsigned int size, Vector2 position, Color color) override;
    void display() override;

private:
    Window &m_window;
    const TextureManager &m_textures;
    const FontManager &m_fonts;
    float m_textScale;
    Vector2 m_offset;
};

} // namespace engine::render
