#pragma once

#include "Entity.hpp"
#include "IComponent.hpp"

namespace engine::ecs {

class Attachment : public IComponent
{
public:
    Attachment(Entity parent, float offsetX, float offsetY);

    Entity parent() const;
    float offsetX() const;
    float offsetY() const;
    bool attached() const;
    void detach();

private:
    Entity m_parent;
    float m_offsetX;
    float m_offsetY;
    bool m_attached;
};

} // namespace engine::ecs
