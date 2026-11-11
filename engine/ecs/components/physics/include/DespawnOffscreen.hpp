#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class DespawnOffscreen : public IComponent
{
public:
    DespawnOffscreen(float minX, float minY, float maxX, float maxY);

    bool isOutside(float x, float y, float width, float height) const;

private:
    float m_minX;
    float m_minY;
    float m_maxX;
    float m_maxY;
};

} // namespace engine::ecs
