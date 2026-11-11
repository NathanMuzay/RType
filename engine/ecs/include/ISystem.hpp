#pragma once

namespace engine::ecs {

class ISystem
{
public:
    virtual ~ISystem() = default;

    virtual void update(float dt) = 0;
};

} // namespace engine::ecs
