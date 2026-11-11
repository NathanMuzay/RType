#pragma once

#include "Entity.hpp"
#include "IComponent.hpp"

namespace engine::ecs {

class Anchor : public IComponent
{
public:
    Anchor(Entity target, float offsetX, float offsetY);

    Entity target() const;
    float offsetX() const;
    float offsetY() const;

private:
    Entity m_target;
    float m_offsetX;
    float m_offsetY;
};

} // namespace engine::ecs
