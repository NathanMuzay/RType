#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class HitFlash : public IComponent
{
public:
    static constexpr float DEFAULT_SECONDS = 0.12f;

    explicit HitFlash(float seconds);

    float remaining() const;
    void elapse(float dt);

private:
    float m_remaining;
};

} // namespace engine::ecs
