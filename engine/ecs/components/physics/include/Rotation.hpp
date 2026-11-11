#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class Rotation : public IComponent
{
public:
    explicit Rotation(float degrees);

    float degrees() const;
    void setDegrees(float degrees);

private:
    float m_degrees;
};

} // namespace engine::ecs
