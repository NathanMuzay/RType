#include "ListRowPart.hpp"

#include <utility>

namespace engine::ecs {

ListRowPart::ListRowPart(std::string prefab, float offsetX, float offsetY)
    : m_prefab(std::move(prefab)), m_offsetX(offsetX), m_offsetY(offsetY)
{
}

const std::string &ListRowPart::prefab() const
{
    return m_prefab;
}

float ListRowPart::offsetX() const
{
    return m_offsetX;
}

float ListRowPart::offsetY() const
{
    return m_offsetY;
}

} // namespace engine::ecs
