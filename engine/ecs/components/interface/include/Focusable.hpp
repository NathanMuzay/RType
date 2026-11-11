#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class Focusable : public IComponent
{
public:
    explicit Focusable(int order);

    int order() const;

private:
    int m_order;
};

} // namespace engine::ecs
