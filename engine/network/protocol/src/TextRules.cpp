#include "TextRules.hpp"

namespace engine::network {

bool TextRules::isValid(const std::string &text, std::size_t minimum, std::size_t maximum)
{
    if (text.size() < minimum || text.size() > maximum)
        return false;
    for (char character : text) {
        if (character < FIRST_PRINTABLE || character > LAST_PRINTABLE)
            return false;
    }
    return true;
}

} // namespace engine::network
