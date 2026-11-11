#pragma once

#include <cstddef>
#include <string>

namespace engine::network {

class TextRules
{
public:
    static constexpr char FIRST_PRINTABLE = ' ';
    static constexpr char LAST_PRINTABLE = '~';

    static bool isValid(const std::string &text, std::size_t minimum, std::size_t maximum);
};

} // namespace engine::network
