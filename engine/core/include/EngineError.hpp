#pragma once

#include <exception>
#include <string>

namespace engine {

class EngineError : public std::exception
{
public:
    explicit EngineError(std::string message);

    const char *what() const noexcept override;

private:
    std::string m_message;
};

} // namespace engine
