#pragma once

#include <cstddef>

namespace engine::ecs {

class ISparseArray
{
public:
    virtual ~ISparseArray() = default;

    virtual void erase(std::size_t index) = 0;
    virtual bool has(std::size_t index) const = 0;
};

} // namespace engine::ecs
