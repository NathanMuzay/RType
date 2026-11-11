#include "Attachment.hpp"

namespace engine::ecs {

Attachment::Attachment(Entity parent, float offsetX, float offsetY)
    : m_parent(parent), m_offsetX(offsetX), m_offsetY(offsetY), m_attached(true)
{
}

Entity Attachment::parent() const
{
    return m_parent;
}

float Attachment::offsetX() const
{
    return m_offsetX;
}

float Attachment::offsetY() const
{
    return m_offsetY;
}

bool Attachment::attached() const
{
    return m_attached;
}

void Attachment::detach()
{
    m_attached = false;
}

} // namespace engine::ecs
