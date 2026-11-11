#pragma once

#include "EngineError.hpp"

namespace engine::net {

class NetError : public EngineError
{
public:
    using EngineError::EngineError;
};

} // namespace engine::net
