#pragma once

#include <string>

#include "IComponent.hpp"

namespace engine::ecs {

class MusicWhen : public IComponent
{
public:
    MusicWhen(std::string key, std::string value, std::string music);

    const std::string &key() const;
    const std::string &value() const;
    const std::string &music() const;

private:
    std::string m_key;
    std::string m_value;
    std::string m_music;
};

} // namespace engine::ecs
