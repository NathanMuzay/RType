#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class Bounded : public IComponent
{
public:
    Bounded(float minX, float minY, float maxX, float maxY);

    float minX() const;
    float minY() const;
    float maxX() const;
    float maxY() const;

private:
    float m_minX;
    float m_minY;
    float m_maxX;
    float m_maxY;
};

} // namespace engine::ecs
