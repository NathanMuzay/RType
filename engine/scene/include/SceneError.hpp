#pragma once

#include "EngineError.hpp"

namespace engine::scene {

class SceneError : public EngineError
{
public:
    using EngineError::EngineError;
};

} // namespace engine::scene
