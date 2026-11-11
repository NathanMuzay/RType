#include "TextFormat.hpp"

namespace engine {

std::string replaceToken(std::string text, const std::string &token, const std::string &value)
{
    if (token.empty())
        return text;
    for (std::size_t position = text.find(token); position != std::string::npos;
         position = text.find(token, position + value.size()))
        text.replace(position, token.size(), value);
    return text;
}

} // namespace engine
