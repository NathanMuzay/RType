#pragma once

#include <string>

#include "Entity.hpp"

namespace engine::ecs {

class IEntityFactory
{
public:
    virtual ~IEntityFactory() = default;

    virtual Entity spawn(const std::string &name) = 0;
};

} // namespace engine::ecs
