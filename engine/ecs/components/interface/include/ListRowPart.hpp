#pragma once

#include <string>

namespace engine::ecs {

class ListRowPart
{
public:
    ListRowPart(std::string prefab, float offsetX, float offsetY);

    const std::string &prefab() const;
    float offsetX() const;
    float offsetY() const;

private:
    std::string m_prefab;
    float m_offsetX;
    float m_offsetY;
};

} // namespace engine::ecs
