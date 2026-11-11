#pragma once

#include "EngineError.hpp"

namespace engine::simulation {

class SimulationError : public EngineError
{
public:
    using EngineError::EngineError;
};

} // namespace engine::simulation
