#pragma once

#include <cstddef>
#include <string>

namespace engine::ecs {

class ConsoleEvent
{
public:
    ConsoleEvent(std::size_t slot, std::string text);

    std::size_t slot() const;
    const std::string &text() const;

private:
    std::size_t m_slot;
    std::string m_text;
};

} // namespace engine::ecs
