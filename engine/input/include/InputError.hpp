#pragma once

#include "EngineError.hpp"

namespace engine::input {

class InputError : public EngineError
{
public:
    using EngineError::EngineError;
};

} // namespace engine::input
