#include "Sprite.hpp"

#include <utility>

namespace engine::ecs {

Sprite::Sprite(std::string texture, IntRect source, float scale)
    : m_texture(std::move(texture)), m_source(source), m_scale(scale)
{
}

const std::string &Sprite::texture() const
{
    return m_texture;
}

IntRect Sprite::source() const
{
    return m_source;
}

void Sprite::setSource(IntRect source)
{
    m_source = source;
}

float Sprite::scale() const
{
    return m_scale;
}

} // namespace engine::ecs
