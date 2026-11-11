#pragma once

#include "EngineError.hpp"

namespace engine {

class PluginError : public EngineError
{
public:
    using EngineError::EngineError;
};

} // namespace engine
