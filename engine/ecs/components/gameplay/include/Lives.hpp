#pragma once

#include "IComponent.hpp"

namespace engine::ecs {

class Lives : public IComponent
{
public:
    explicit Lives(int count);

    int count() const;
    void setCount(int count);
    void lose();
    void reset();

private:
    int m_start;
    int m_count;
};

} // namespace engine::ecs
