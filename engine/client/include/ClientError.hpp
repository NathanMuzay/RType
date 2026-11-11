#pragma once

#include "EngineError.hpp"

namespace engine::client {

class ClientError : public EngineError
{
public:
    using EngineError::EngineError;
};

} // namespace engine::client
