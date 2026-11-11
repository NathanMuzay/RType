#pragma once

#include <string>
#include <unordered_map>

#include <SFML/Graphics/Texture.hpp>

namespace engine::render {

class TextureManager
{
public:
    void declare(const std::string &name, const std::string &path);
    bool declared(const std::string &name) const;
    void preload(const std::string &name) const;
    const sf::Texture &get(const std::string &name) const;

private:
    std::unordered_map<std::string, std::string> m_paths;
    mutable std::unordered_map<std::string, sf::Texture> m_textures;
};

} // namespace engine::render
