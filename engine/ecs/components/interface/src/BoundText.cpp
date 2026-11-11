#include "BoundText.hpp"

#include <utility>

namespace engine::ecs {

BoundText::BoundText(std::string format) : m_format(std::move(format))
{
}

const std::string &BoundText::format() const
{
    return m_format;
}

} // namespace engine::ecs
