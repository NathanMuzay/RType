#pragma once

#include "EngineError.hpp"

namespace engine {

class CoreError : public EngineError
{
public:
    using EngineError::EngineError;
};

} // namespace engine
