#pragma once

#include "EngineError.hpp"

namespace engine::audio {

class AudioError : public EngineError
{
public:
    using EngineError::EngineError;
};

} // namespace engine::audio
