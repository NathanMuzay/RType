#include "EngineError.hpp"

#include <utility>

namespace engine {

EngineError::EngineError(std::string message) : m_message(std::move(message))
{
}

const char *EngineError::what() const noexcept
{
    return m_message.c_str();
}

} // namespace engine
