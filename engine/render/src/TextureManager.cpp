#include "TextureManager.hpp"

#include <utility>

#include "RenderError.hpp"

namespace engine::render {

void TextureManager::declare(const std::string &name, const std::string &path)
{
    m_paths.insert_or_assign(name, path);
    m_textures.erase(name);
}

bool TextureManager::declared(const std::string &name) const
{
    return m_paths.contains(name);
}

void TextureManager::preload(const std::string &name) const
{
    get(name);
}

const sf::Texture &TextureManager::get(const std::string &name) const
{
    const auto loaded = m_textures.find(name);
    if (loaded != m_textures.end())
        return loaded->second;
    const auto path = m_paths.find(name);
    if (path == m_paths.end())
        throw RenderError("TextureManager: unknown texture " + name);
    sf::Texture texture;
    if (!texture.loadFromFile(path->second))
        throw RenderError("TextureManager: cannot load " + path->second);
    return m_textures.insert_or_assign(name, std::move(texture)).first->second;
}

} // namespace engine::render
