#pragma once

#include "EngineError.hpp"

namespace engine::scripting {

class ScriptError : public EngineError
{
public:
    using EngineError::EngineError;
};

} // namespace engine::scripting
