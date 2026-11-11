#pragma once

namespace engine::ecs {

class IComponent
{
protected:
    IComponent() = default;
    ~IComponent() = default;
    IComponent(const IComponent &) = default;
    IComponent(IComponent &&) = default;
    IComponent &operator=(const IComponent &) = default;
    IComponent &operator=(IComponent &&) = default;
};

} // namespace engine::ecs
