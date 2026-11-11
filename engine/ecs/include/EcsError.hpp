#pragma once

#include "EngineError.hpp"

namespace engine::ecs {

class EcsError : public EngineError
{
public:
    using EngineError::EngineError;
};

} // namespace engine::ecs
