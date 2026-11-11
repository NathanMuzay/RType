#include "SoundEvent.hpp"

#include <utility>

namespace engine::ecs {

SoundEvent::SoundEvent(std::string name) : m_name(std::move(name))
{
}

const std::string &SoundEvent::name() const
{
    return m_name;
}

} // namespace engine::ecs
