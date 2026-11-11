#include "FontManager.hpp"

#include <utility>

#include "RenderError.hpp"

namespace engine::render {

void FontManager::declare(const std::string &name, const std::string &path)
{
    m_paths.insert_or_assign(name, path);
    m_fonts.erase(name);
}

const sf::Font &FontManager::get(const std::string &name) const
{
    const auto loaded = m_fonts.find(name);
    if (loaded != m_fonts.end())
        return loaded->second;
    const auto path = m_paths.find(name);
    if (path == m_paths.end())
        throw RenderError("FontManager: unknown font " + name);
    sf::Font font;
    if (!font.openFromFile(path->second))
        throw RenderError("FontManager: cannot load " + path->second);
    return m_fonts.insert_or_assign(name, std::move(font)).first->second;
}

} // namespace engine::render
