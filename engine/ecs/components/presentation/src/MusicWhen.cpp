#include "MusicWhen.hpp"

#include <utility>

namespace engine::ecs {

MusicWhen::MusicWhen(std::string key, std::string value, std::string music)
    : m_key(std::move(key)), m_value(std::move(value)), m_music(std::move(music))
{
}

const std::string &MusicWhen::key() const
{
    return m_key;
}

const std::string &MusicWhen::value() const
{
    return m_value;
}

const std::string &MusicWhen::music() const
{
    return m_music;
}

} // namespace engine::ecs
