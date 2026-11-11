#pragma once

#include "EngineError.hpp"

namespace engine::render {

class RenderError : public EngineError
{
public:
    using EngineError::EngineError;
};

} // namespace engine::render
