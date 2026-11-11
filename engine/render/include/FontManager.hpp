#pragma once

#include <string>
#include <unordered_map>

#include <SFML/Graphics/Font.hpp>

namespace engine::render {

class FontManager
{
public:
    void declare(const std::string &name, const std::string &path);
    const sf::Font &get(const std::string &name) const;

private:
    std::unordered_map<std::string, std::string> m_paths;
    mutable std::unordered_map<std::string, sf::Font> m_fonts;
};

} // namespace engine::render
