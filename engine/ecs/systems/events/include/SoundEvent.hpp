#pragma once

#include <string>

namespace engine::ecs {

class SoundEvent
{
public:
    explicit SoundEvent(std::string name);

    const std::string &name() const;

private:
    std::string m_name;
};

} // namespace engine::ecs
