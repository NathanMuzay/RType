#include "SfmlRenderer.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

namespace engine::render {

namespace {

sf::Color toSfml(Color color)
{
    return sf::Color(color.r(), color.g(), color.b(), color.a());
}

} // namespace

SfmlRenderer::SfmlRenderer(Window &window, const TextureManager &textures, const FontManager &fonts)
    : m_window(window), m_textures(textures), m_fonts(fonts), m_textScale(1.f), m_offset()
{
}

void SfmlRenderer::setTextScale(float scale)
{
    if (scale > 0.f)
        m_textScale = scale;
}

void SfmlRenderer::clear(Color color)
{
    m_window.native().clear(toSfml(color));
}

void SfmlRenderer::drawRect(Vector2 position, Vector2 size, Color color, Color outline, float thickness)
{
    sf::RectangleShape rect({size.x(), size.y()});
    rect.setPosition({position.x() + m_offset.x(), position.y() + m_offset.y()});
    rect.setFillColor(toSfml(color));
    if (thickness > 0.f) {
        rect.setOutlineColor(toSfml(outline));
        rect.setOutlineThickness(thickness);
    }
    m_window.native().draw(rect);
}

void SfmlRenderer::drawTexture(
    const std::string &texture, IntRect source, Vector2 position, Vector2 scale, float rotation, Color tint)
{
    sf::Sprite sprite(m_textures.get(texture));
    sprite.setTextureRect(sf::IntRect({source.x(), source.y()}, {source.width(), source.height()}));
    const float halfWidth = static_cast<float>(source.width()) / 2.f;
    const float halfHeight = static_cast<float>(source.height()) / 2.f;
    sprite.setOrigin({halfWidth, halfHeight});
    sprite.setPosition(
        {position.x() + m_offset.x() + halfWidth * scale.x(), position.y() + m_offset.y() + halfHeight * scale.y()});
    sprite.setScale({scale.x(), scale.y()});
    sprite.setRotation(sf::degrees(rotation));
    sprite.setColor(toSfml(tint));
    m_window.native().draw(sprite);
}

void SfmlRenderer::setOffset(Vector2 offset)
{
    m_offset = offset;
}

Vector2 SfmlRenderer::offset() const
{
    return m_offset;
}

void SfmlRenderer::drawText(
    const std::string &font, const std::string &text, unsigned int size, Vector2 position, Color color)
{
    sf::Text label(m_fonts.get(font), text, static_cast<unsigned int>(static_cast<float>(size) * m_textScale));
    label.setPosition({position.x(), position.y()});
    label.setFillColor(toSfml(color));
    m_window.native().draw(label);
}

void SfmlRenderer::display()
{
    m_window.native().display();
}

} // namespace engine::render
